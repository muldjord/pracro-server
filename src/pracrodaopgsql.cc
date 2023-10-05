/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            pracrodaopgsql.cc
 *
 *  Wed Feb 11 11:18:26 CET 2009
 *  Copyright 2009 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Pracro.
 *
 *  Pracro is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Pracro is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Pracro; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "pracrodaopgsql.h"

/*
 * Updating the old tables;
 *
 * ALTER TABLE transactions ADD COLUMN uid bigint;
 * CREATE SEQUENCE 'trseq';
 * SELECT setval('trseq', (SELECT MAX(oid) FROM transactions));
 * UPDATE transactions SET uid = oid;
 * INSERT INTO fieldnames (name, description, timestamp) 
 *   VALUES ('journal.resume', 'Journal resume text', 
 *   (SELECT EXTRACT(EPOCH FROM now())::integer));
 */
#include <config.h>

#ifndef WITHOUT_DB

#include <stdlib.h>
#include <list>
#include <sstream>

#include <hugin.hpp>

#include "pgwork.h"

PracroDAOPgsql::PracroDAOPgsql(std::string _host, std::string _port,
                               std::string _user, std::string _passwd,
                               std::string _dbname)
  : PracroDAO(_host, _port, _user, _passwd, _dbname)
{
  conn = nullptr;
  std::string cs;
  if(host.size()) cs += " host=" + host;
  if(port.size()) cs += " port=" + port;
  if(user.size()) cs += " user=" + user;
  if(passwd.size()) cs += " password=" + passwd;
  cs += " dbname=" + (dbname.size() ? dbname : "pracro");

  conn = PQconnectdb(cs.c_str());

  if(conn == nullptr || PQstatus(conn) == CONNECTION_BAD)	{
    ERR(db, "Postgresql init failed: %s\n", PQerrorMessage(conn));
    conn = nullptr;
    return;
  }

  DEBUG(db, "Pgsql connection %p (%s)\n", conn, cs.c_str());
}

PracroDAOPgsql::~PracroDAOPgsql()
{
  if(conn) {
    PQfinish(conn);
  }
}

std::string PracroDAOPgsql::newSessionId()
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return "";
  }

  try {
    Work W(conn);
    result_t R = W.exec("SELECT nextval('sessionseq');");
    result_t::const_iterator ri = R.begin();
    if(ri != R.end()) {
      DEBUG(db, "New session id: %s\n", (*ri)[0].c_str());
      return (*ri)[0].c_str();
    }

    ERR(db, "Something wrong with the session counter.\n");

  } catch(std::exception &e) {
    ERR(db, "Session counter failed: %s\n", e.what());
  }

  return "";
}

void PracroDAOPgsql::commitTransaction(std::string sessionid,
                                       Transaction &transaction,
                                       Commit &commit,
                                       Macro &_macro,
                                       time_t now)
{
  DEBUG(db, "commitTransaction (%s, %s, %s, <%d fields>, %u)\n",
        transaction.user.c_str(), transaction.patientid.c_str(),
        _macro.name.c_str(),
        (int)commit.fields.size(), (unsigned int)now);
  
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  if(commit.fields.size() == 0) return;

  Work W(conn);

  std::string version = _macro.version;
  std::string macro = _macro.name;
  std::stringstream timestamp; timestamp << now;

  std::string ts;

  try {
    ts = "SELECT status FROM commits WHERE uid='"+sessionid+"';";
    result_t R = W.exec(ts);
    if(!R.size()) {
      ts = "INSERT INTO commits (patientid, template, version,"
        " \"timestamp\", uid, status) VALUES ("
        " '" + W.esc(transaction.patientid) + "', "
        " '" + W.esc(commit.templ) + "', "
        " '" + "1.0" + "', "
        " '" + W.esc(timestamp.str()) + "', "
        " '" + W.esc(sessionid) + "', "
        " 'active' "
        ");"
        ;
      DEBUG(sql, "Query: %s\n", ts.c_str());
      result_t R = W.exec(ts);
    } else {
      
      result_t::const_iterator ri = R.begin();
      if(ri != R.end()) {
        std::string status = (*ri)[0].c_str();
        if(status == "committed") {
          ERR(db, "Attempt to add to committed session %s blocked!\n",
                  sessionid.c_str());
          return;
        }
      }
      
      ts = "UPDATE commits SET status='active' WHERE uid="+sessionid+";";
      DEBUG(sql, "Query: %s\n", ts.c_str());
      /*result_t R = */W.exec(ts);
    }
  } catch(std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), ts.c_str());
    return;
  }

  try {
    ts = "INSERT INTO transactions (uid, macro, version,"
      " \"timestamp\", \"user\", cid) VALUES ("
      " nextval('trseq'), "
      " '" + W.esc(macro) + "', "
      " '" + W.esc(version) + "', "
      " '" + W.esc(timestamp.str()) + "', "
      " '" + W.esc(transaction.user) + "', "
      " '" + W.esc(sessionid) + "' "
      ");"
      ;
    DEBUG(sql, "Query: %s\n", ts.c_str());
    result_t R = W.exec(ts);

    if(commit.fields.size() > 0) {
      // field table lookup
      ts = "SELECT DISTINCT name FROM fieldnames WHERE name IN ( ";
      std::map< std::string, std::string >::iterator i = commit.fields.begin();
      ts += "'" + W.esc(i->first) + "'";
      i++;
      while(i != commit.fields.end()) {
        ts += ", '" + W.esc(i->first) + "'";
        i++;
      }
      ts += ");";
      DEBUG(sql, "Query: %s\n", ts.c_str());
      R = W.exec(ts);

      DEBUG(db, "input fields: %d, output fields: %d\n",
            (int)commit.fields.size(), (int)R.size());

      // Store known fields
      result_t::const_iterator ri = R.begin();
      if(ri != R.end()) {
        std::string name = (*ri)[0].c_str();
        DEBUG(db, "Storing: %s with value %s\n",
              name.c_str(), commit.fields[name].c_str());
        ts = "INSERT INTO fields (transaction, name, value) "
          "VALUES ( currval('trseq'), '" + W.esc(name) + "', '" +
          W.esc(commit.fields[name]) + "')";
        ri++;
        while(ri != R.end()) {
          name = (*ri)[0].c_str();

          DEBUG(db, "Storing: %s with value %s\n",
                name.c_str(), commit.fields[name].c_str());

          ts += ", (currval('trseq'), '" + W.esc(name) + "', '" +
            W.esc(commit.fields[name]) + "')";
          ri++;
        }
        ts += ";";
        DEBUG(sql, "Query: %s\n", ts.c_str());
        W.exec(ts);
      }
    }

    W.commit();

  } catch(std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), ts.c_str());
  }

}

//#define NEW
Values PracroDAOPgsql::getLatestValues(std::string sessionid,
                                       std::string patientid,
                                       Macro *macro,
                                       Fieldnames &fieldnames,
                                       time_t oldest)
{
  Values values;

  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return values;
  }

  bool uncom = false; // get results that are not yet committed?

  DEBUG(db, "(%s, %s, <%d fieldnames>, %u)\n",
        patientid.c_str(),
        macro ? macro->name.c_str() : "(null)",
        (int)fieldnames.size(), (unsigned int)oldest);

  std::string query;
  std::stringstream soldest; soldest << oldest;
  try {
    {
      Work W(conn);
      query = "UPDATE commits SET status='active' WHERE status='idle'"
        " AND uid="+sessionid+";";
      DEBUG(sql, "Query: %s\n", query.c_str());
      /*result_t R = */W.exec(query);
      W.commit();
    }

    Work W(conn);

#ifdef NEW

    // Do not search for nothing...
    if(fieldnames.size() == 0) return values;

    std::string names;
    std::vector< std::string >::iterator fni = fieldnames.begin();
    while(fni != fieldnames.end()) {
      if(names != "") names += " OR ";
      names += "name='" + W.esc(*fni) + "'";
      fni++;
    }

    std::string macros;
    if(macro) {
      macros += " AND macro='" + macro->name + "'";
      if(macro->version != "")
        macros += " AND t.version='" + macro->version + "'";
    }

    uncom = uncom;
    query = "SELECT uid FROM commits WHERE patientid='"+patientid+"' AND"
      " \"timestamp\">="+soldest.str()+" AND"
      " (status='committed' OR uid="+sessionid+");";
    DEBUG(sql, "Query: %s\n", query.c_str());
    result_t commits = W.exec(query);
    result_t::const_iterator ci = commits.begin();
    while(ci != commits.end()) {
      std::string cid = (*ci)[0].c_str();

      query = "SELECT uid, \"timestamp\" FROM transactions WHERE cid="+cid+
        macros+";";
      DEBUG(sql, "Query: %s\n", query.c_str());
      result_t transactions = W.exec(query);
      result_t::const_iterator ti = transactions.begin();
      while(ti != transactions.end()) {
        std::string tid = (*ti)[0].c_str();
        time_t timestamp = atol((*ti)[1].c_str());

        query = "SELECT name, value FROM fields WHERE"
          " transaction="+tid+" AND ("+ names +");";
        DEBUG(sql, "Query: %s\n", query.c_str());
        result_t fields = W.exec(query);
        DEBUG(sql, "Results: %lu\n", fields.size());
        result_t::const_iterator fi = fields.begin();
        while(fi != fields.end()) {
          std::string name = (*fi)[0].c_str();
          if(values.find(name) == values.end() ||
             values[name].timestamp <= timestamp) {
            Value v;
            v.value = (*fi)[1].c_str();
            v.timestamp = timestamp;
            values[name] = v;
          }
          fi++;
        }

        ti++;
      }

      ci++;
    }
#else/*NEW*/
    std::string namecond;

    if(fieldnames.size() > 0) {
      std::vector< std::string >::iterator i = fieldnames.begin();
      namecond += " AND f.name IN ('" + W.esc(*i) + "'";
      i++;
      while(i != fieldnames.end()) {
        namecond += ", '" + W.esc(*i) + "'";
        i++;
      }
      namecond += ')';
    }
    query = "SELECT ff.name, ff.value, tt.timestamp FROM "
    // Begin inner query
      " (SELECT f.name, MAX(t.timestamp) AS ts "
      "   FROM commits c, fields f, transactions t "
      "   WHERE ";
    if(!uncom) {
      query += "((c.status='committed' AND t.timestamp >= " + soldest.str() +
        ") OR c.uid="+sessionid+") AND ";
    }
    query += "c.uid = t.cid AND t.uid = f.transaction"
      // " AND t.timestamp >= " + soldest.str() +
      " AND c.patientid = '" + W.esc(patientid) + "' "
      + namecond;
    if(macro) {
      query += " AND t.macro = '" + macro->name + "'";
      if(macro->version != "")
        query += " AND t.version = '" + macro->version + "'";
    }
    query += " GROUP BY f.name) xx, "
    // End inner query
      " transactions tt, fields ff, commits cc "
      " WHERE ";
    if(!uncom) query += "(cc.status='committed' OR cc.uid="+sessionid+") AND ";
    query += " xx.ts = tt.timestamp "
      "   AND xx.name = ff.name "
      "   AND tt.uid = ff.transaction "
      "   AND tt.cid = cc.uid "
      "   AND cc.patientid = '" + W.esc(patientid) + "' "
      ;
    if(macro) {
      query += " AND tt.macro = '" + macro->name + "'";
      if(macro->version != "")
        query += " AND tt.version = '" + macro->version + "'";
    }

    DEBUG(sql, "Query: %s\n", query.c_str());
    result_t R = W.exec(query);
    result_t::const_iterator ri = R.begin();
    while(ri != R.end()) {
      Value v;
      v.value = (*ri)[1].c_str();
      v.timestamp = atol((*ri)[2].c_str());
      values[(*ri)[0].c_str()] = v;
      ri++;
    }
#endif/*NEW*/
  } catch (std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), query.c_str());
  }

  return values;
}


unsigned PracroDAOPgsql::nrOfCommits(std::string sessionid,
                                     std::string patientid,
                                     std::string macroname,
                                     time_t oldest)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return 0;
  }

  bool uncom = false; // get results that are not yet committed?

  std::string query;
  std::stringstream soldest; soldest << oldest;
  try {
    Work W(conn);
    query = "SELECT count(*) FROM commits c, transactions f"
      " WHERE c.patientid = '" + W.esc(patientid) + "' AND c.uid = f.cid";
    //if(!uncom) query += " AND (c.status='committed' OR c.uid="+sessionid+")";
    if(!uncom) query += " AND ((c.status='committed' AND f.timestamp >= " + soldest.str() + ") OR c.uid="+sessionid+")";
    query += " AND f.macro = '" + W.esc(macroname) + "' "
      //" AND f.timestamp >= " + soldest.str()
      ;
    DEBUG(sql, "Query: %s\n", query.c_str());
    result_t R = W.exec(query);
    if(R.size() != 1) {
      ERR(db, "No result set; expected one row with one column\n");
      return 0;
    }
    unsigned n = (unsigned)atol((*R.begin())[0].c_str());
    DEBUG(db, "Found %u commits for %s(%s) from %ld\n",
                 n, patientid.c_str(), macroname.c_str(), oldest);
    return n;
  } catch (std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), query.c_str());
  }

  return 0;
}

void PracroDAOPgsql::addFieldname(std::string name, std::string description)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  std::stringstream timestamp; timestamp << time(nullptr);
  std::string ts;

  try {
    Work W(conn);

    ts = "SELECT name FROM fieldnames WHERE name='"+W.esc(name)+"';";
    result_t Rc = W.exec(ts);
    if(Rc.size()) {
      ts = "UPDATE fieldnames SET "
        " description='" + W.esc(description) + "', "
        " WHERE name='" + W.esc(name) + "';"
        ;
    } else {
      ts = "INSERT INTO fieldnames (name, description, \"timestamp\") VALUES ("
        " '" + W.esc(name) + "', "
        " '" + W.esc(description) + "', "
        " '" + W.esc(timestamp.str()) + "' "
        ");"
        ;
    }
    DEBUG(sql, "Query: %s\n", ts.c_str());
    result_t R = W.exec(ts);
    W.commit();
  } catch (std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), ts.c_str());
  }
}

void PracroDAOPgsql::delFieldname(std::string name)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  std::string ts;
  try {
    Work W(conn);
    ts = "DELETE FROM fieldnames WHERE name="
      "'" + W.esc(name) + "' ";
    DEBUG(sql, "Query: %s\n", ts.c_str());
    result_t R = W.exec(ts);
    W.commit();
  } catch (std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), ts.c_str());
  }
}

std::vector<Fieldname> PracroDAOPgsql::getFieldnames()
{
  std::vector<Fieldname> fieldnames;

  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return fieldnames;
  }

  std::string query;
  try {
    Work W(conn);
    query = "SELECT * FROM fieldnames";
    DEBUG(sql, "Query: %s\n", query.c_str());
    result_t R = W.exec(query);
    result_t::const_iterator ri = R.begin();
    while(ri != R.end()) {
      Fieldname f;
      f.name = (*ri)[0].c_str();
      f.description = (*ri)[1].c_str();
      f.timestamp = atol((*ri)[2].c_str());
      fieldnames.push_back(f);
      ri++;
    }
  } catch (std::exception &e) {
    ERR(db, "Query failed: %s: %s\n", e.what(), query.c_str());
  }

  return fieldnames;
}

void PracroDAOPgsql::commit(std::string sessionid)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  std::string ts;
  try {
    Work W(conn);
    ts = "UPDATE commits SET status='committed' WHERE uid="+sessionid+";";
    /*result_t R = */W.exec(ts);
    
    W.commit();
  } catch (std::exception &e) {
    ERR(db, "Commit failed: %s: %s\n", e.what(), ts.c_str());
  }
}

void PracroDAOPgsql::nocommit(std::string sessionid)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  std::string ts;
  try {
    Work W(conn);
    ts = "UPDATE commits SET status='idle' WHERE uid="+sessionid+";";
    /*result_t R = */W.exec(ts);
    
    W.commit();
  } catch (std::exception &e) {
    ERR(db, "NoCommit failed: %s: %s\n", e.what(), ts.c_str());
  }
}

void PracroDAOPgsql::discard(std::string sessionid)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  std::string ts;
  try {
    Work W(conn);
    ts = "DELETE FROM commits WHERE uid="+sessionid+";";
    /*result_t R = */W.exec(ts);
    W.commit();
  } catch (std::exception &e) {
    ERR(db, "Abort (rollback) failed: %s: %s\n", e.what(), ts.c_str());
  }
}

bool PracroDAOPgsql::idle(std::string sessionid)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return false;
  }

  std::string ts = "SELECT status FROM commits WHERE uid='"+sessionid+"';";
  try {
    Work W(conn);
    result_t R = W.exec(ts);
    result_t::const_iterator ri = R.begin();
    if(ri != R.end()) {
      std::string status = (*ri)[0].c_str();
      return status == "idle";
    }
  } catch (std::exception &e) {
    ERR(db, "setIdle failed: %s: %s\n", e.what(), ts.c_str());
  }

  return false;
}

void PracroDAOPgsql::setIdle(std::string sessionid, bool idle)
{
  if(!conn) {
    ERR(db, "No pgsql connection\n");
    return;
  }

  std::string ts;
  try {
    Work W(conn);
    if(idle) {
      ts = "UPDATE commits SET status='idle' WHERE uid="+sessionid+
        " AND status='active';";
    } else {
      ts = "UPDATE commits SET status='active' WHERE uid="+sessionid+
        " AND status='idle';";
    }
    /*result_t R = */W.exec(ts);
    
    W.commit();
  } catch (std::exception &e) {
    ERR(db, "setIdle failed: %s: %s\n", e.what(), ts.c_str());
  }

}

#endif/*WITHOUT_DB*/

#ifdef TEST_PRACRODAOPGSQL
//deps: debug.cc log.cc configuration.cc exception.cc pracrodao.cc mutex.cc
//cflags: -I.. $(PQXX_CXXFLAGS) $(PTHREAD_CFLAGS)
//libs: $(PQXX_LIBS) $(PTHREAD_LIBS)
#include <test.h>
#include <time.h>

#define PATIENTID "1234567890"
#define MACRO "testmacro"

static bool vectorFind(std::vector<Fieldname> fs,
                       std::string name, std::string desc)
{
  std::vector<Fieldname>::iterator i = fs.begin();
  while(i != fs.end()) {
    Fieldname &fn = *i;
    if(fn.name == name && 
       (desc == "" || fn.description == desc)) return true;
    i++;
  }
  return false;
}

TEST_BEGIN;

debug_parse("+all");

PracroDAOPgsql db("localhost", "", "pracro", "pracro", "pracrotest");

db.addFieldname("field1", "desc1");
db.addFieldname("field2", "desc2");
db.addFieldname("field3", "desc3");
db.delFieldname("field3");

std::vector<Fieldname> fs = db.getFieldnames();
TEST_EQUAL_INT(fs.size(), 2, "Test fieldname size.");
TEST_TRUE(vectorFind(fs, "field1", "desc1"), "Test fieldname 'field1'.");
TEST_TRUE(vectorFind(fs, "field2", "desc2"), "Test fieldname 'field2'.");
TEST_FALSE(vectorFind(fs, "field3", ""), "Test fieldname 'field3'.");

std::string sid1 = db.newSessionId();
std::string sid2 = db.newSessionId();

TEST_NOTEQUAL_STR(sid1, sid2, "Do not produce the same uid each time.");

Transaction transaction;
transaction.patientid = PATIENTID;
transaction.user = "me";

Commit commit;
commit.fields["field1"] = "hello";
commit.fields["field2"] = "world";
commit.templ = "tester";

Macro macro;
macro.version = "1.0";
macro.name = MACRO;

time_t now = time(nullptr);

db.commitTransaction(sid1, transaction, commit, macro, now);

TEST_EQUAL_INT(db.nrOfCommits(sid1, PATIENTID, MACRO, now), 1, "How many?");

Fieldnames fieldnames;
fieldnames.push_back("field1");
fieldnames.push_back("field_nop");
Values vals = db.getLatestValues(sid1, PATIENTID, &macro, fieldnames, 0);
TEST_EQUAL_INT(vals.size(), 1, "One value");

TEST_NOTEQUAL(vals.find("field1"), vals.end(), "find value");

{
  std::string sid = db.newSessionId();
  db.commitTransaction(sid, transaction, commit, macro, now);
  TEST_FALSE(db.idle(sid), "Session should not be idle.");

  db.setIdle(sid, true);
  TEST_TRUE(db.idle(sid), "Session should be idle.");

  db.setIdle(sid, false);
  TEST_FALSE(db.idle(sid), "Session1 should not be idle.");
}

{
  std::string sid = db.newSessionId();
  db.commitTransaction(sid, transaction, commit, macro, now);
  TEST_FALSE(db.idle(sid), "Session should not be idle.");
  db.commit(sid);
  TEST_FALSE(db.idle(sid), "Session is not idle (since committed != idle).");
}

{
  std::string sid = db.newSessionId();
  db.commitTransaction(sid, transaction, commit, macro, now);
  TEST_FALSE(db.idle(sid), "Session should not be idle.");
  db.nocommit(sid);
  TEST_TRUE(db.idle(sid), "Session is idle.");
}

{
  std::string sid = db.newSessionId();
  db.commitTransaction(sid, transaction, commit, macro, now);
  TEST_FALSE(db.idle(sid), "Session should not be idle.");
  db.discard(sid);
  TEST_FALSE(db.idle(sid), "Session not idle (it doesn't exist).");
}

TEST_FALSE(db.idle("no such session"), "Missing session is not idle.");

{
  Commit commit;
  commit.templ = "tester";

  std::string sid = db.newSessionId();

  commit.fields["field1"] = "hello";
  commit.fields["field2"] = "world";
  db.commitTransaction(sid, transaction, commit, macro, now + 1);

  commit.fields["field1"] = "hello2";
  commit.fields["field2"] = "world2";
  db.commitTransaction(sid, transaction, commit, macro, now + 2);

  Fieldnames fieldnames;
  fieldnames.push_back("field1");
  fieldnames.push_back("field2");
  Values vals = db.getLatestValues(sid, PATIENTID, &macro, fieldnames, 0);
  TEST_EQUAL_STR(vals["field1"].value, "hello2", "Latest one only please");
  TEST_EQUAL_STR(vals["field2"].value, "world2", "Latest one only please");
}

{
  Commit commit;
  commit.templ = "tester";

  std::string sid = db.newSessionId();

  commit.fields["field1"] = "hello1";
  commit.fields["field2"] = "world1";
  db.commitTransaction(sid, transaction, commit, macro, now + 4);

  commit.fields["field1"] = "hello2";
  commit.fields["field2"] = "world2";
  db.commitTransaction(sid, transaction, commit, macro, now + 3);

  Fieldnames fieldnames;
  fieldnames.push_back("field1");
  fieldnames.push_back("field2");
  Values vals = db.getLatestValues(sid, PATIENTID, &macro, fieldnames, 0);
  TEST_EQUAL_STR(vals["field1"].value, "hello1", "Latest one only please");
  TEST_EQUAL_STR(vals["field2"].value, "world1", "Latest one only please");
}

{
  Commit commit;
  commit.templ = "tester";

  std::string sid = db.newSessionId();

  commit.fields["field1"] = "hello3";
  commit.fields["field2"] = "world3";
  db.commitTransaction(sid, transaction, commit, macro, now + 5);

  db.commit(sid);

  commit.fields["field1"] = "hello4";
  commit.fields["field2"] = "world4";
  db.commitTransaction(sid, transaction, commit, macro, now + 6);

  Fieldnames fieldnames;
  fieldnames.push_back("field1");
  fieldnames.push_back("field2");
  Values vals = db.getLatestValues(sid, PATIENTID, &macro, fieldnames, 0);
  TEST_EQUAL_STR(vals["field1"].value, "hello3", "Latest one only please");
  TEST_EQUAL_STR(vals["field2"].value, "world3", "Latest one only please");
}

{ // Only see values if they are from your own session or committed.
  Commit commit;
  commit.templ = "tester";

  std::string sid1 = db.newSessionId();
  std::string sid2 = db.newSessionId();

  commit.fields["field1"] = "hello1";
  commit.fields["field2"] = "world1";
  db.commitTransaction(sid1, transaction, commit, macro, now + 7);

  commit.fields["field1"] = "hello2";
  commit.fields["field2"] = "world2";
  db.commitTransaction(sid2, transaction, commit, macro, now + 6);

  Fieldnames fieldnames;
  fieldnames.push_back("field1");
  fieldnames.push_back("field2");
  {
    Values vals = db.getLatestValues(sid2, PATIENTID, &macro, fieldnames, 0);
    TEST_EQUAL_STR(vals["field1"].value, "hello2", "Latest one only please");
    TEST_EQUAL_STR(vals["field2"].value, "world2", "Latest one only please");
  }

  db.commit(sid1);

  {
    Values vals = db.getLatestValues(sid2, PATIENTID, &macro, fieldnames, 0);
    TEST_EQUAL_STR(vals["field1"].value, "hello1", "Latest one only please");
    TEST_EQUAL_STR(vals["field2"].value, "world1", "Latest one only please");
  }
}

{
  Commit commit;
  commit.templ = "tester";

  std::string sid = db.newSessionId();

  commit.fields["foo"] = "hello";
  commit.fields["bar"] = "world";
  db.commitTransaction(sid, transaction, commit, macro, now);

  Fieldnames fieldnames;
  fieldnames.push_back("foo");
  fieldnames.push_back("bar");
  Values vals = db.getLatestValues(sid, PATIENTID, &macro, fieldnames, 0);
  TEST_EQUAL_INT(vals.size(), 0, "Fields did not exists");
}

{ // Lets test those ERRORS
  PracroDAOPgsql db("localhost", "", "pracro", "pracro", "no_such_db");

  TEST_EQUAL_STR(db.newSessionId(), "", "Don't get session id.");

  // Just don't crash on this one...
  Commit commit;
  db.commitTransaction("", transaction, commit, macro, now);

  // Again don't crash.
  Fieldnames fieldnames;
  Values vals = db.getLatestValues("", PATIENTID, &macro, fieldnames, 0);
  TEST_EQUAL_INT(vals.size(), 0, "Don't get any values.");
  
  // Don't crash here either...
  db.commit("");
  db.nocommit("");
  db.discard("");
  TEST_FALSE(db.idle(""), "no connection == not idle");
  db.setIdle("", true);
  TEST_EQUAL_INT(db.nrOfCommits("", "", "", 0), 0, "We should get 0 commits.");

  // And again; no crash.
  db.addFieldname("", "");
  db.delFieldname("");
  std::vector<Fieldname, std::allocator<Fieldname> > f = db.getFieldnames();
  TEST_EQUAL_INT(f.size(), 0, "No fieldnames");
}

TEST_END;

#endif/*TEST_PRACRODAOPGSQL*/

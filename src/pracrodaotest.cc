/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            pracrodaotest.cc
 *
 *  Fri Aug  7 10:25:07 CEST 2009
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
#include "pracrodaotest.h"

#include <stdlib.h>
#include <sstream>

#include <hugin.hpp>

static dbtable_t::iterator select(dbtable_t &table,
                                  std::string key, std::string value)
{
  dbtable_t::iterator i = table.begin();
  while(i != table.end()) {
    dbrow_t &r = *i;
    if(r.find(key) != r.end() && r[key] == value) return i;
    i++;
  }

  return i;
}


PracroDAOTest::PracroDAOTest(bool ignore_fieldnames)
  : PracroDAO("", "", "", "", "")
{
  this->ignore_fieldnames = ignore_fieldnames;
  DEBUG(db, "New test (memory only) database\n");
}

PracroDAOTest::~PracroDAOTest()
{
  DEBUG(db, "Delete test (memory only) database\n");
}

std::string PracroDAOTest::newSessionId()
{
  return data.sessionseq.nextval();
}

void PracroDAOTest::commitTransaction(std::string sessionid,
                                      Transaction &transaction,
                                      Commit &commit,
                                      Macro &_macro,
                                      time_t now)
{
  DEBUG(db, "(%s, %s, %s, <%u fields>, %ld)\n",
        transaction.user.c_str(),
        transaction.patientid.c_str(),
        _macro.name.c_str(),
        (int)commit.fields.size(),
        now);
  if(commit.fields.size() == 0) return;

  std::string version = _macro.version;
  std::string macro = _macro.name;
  std::stringstream timestamp; timestamp << now;

  dbtable_t::iterator ci = select(data.commits, "uid", sessionid);
 
  if(ci == data.commits.end()) {
    DEBUG(db, "Create new commit: %s\n", sessionid.c_str());
    dbrow_t c;
    c["patientid"] = transaction.patientid;
    c["template"] = commit.templ;
    c["version"] = "1.0";
    c["timestamp"] = timestamp.str();
    c["uid"] = sessionid;
    c["status"] = "active";
    data.commits.push_back(c);
    ci = select(data.commits, "uid", sessionid);//data.commits.rbegin();
  } else {
    dbrow_t &c = *ci;
    if(c["status"] == "committed") {
      ERR(db, "Attempt to resume committed session %s blocked!\n",
              sessionid.c_str());
      return;
    }

    DEBUG(db, "Resuming commit: %s\n", sessionid.c_str());
    c["status"] = "active";
  }

  dbrow_t &c = *ci;

  dbrow_t t;
  t["uid"] = data.trseq.nextval();
  t["macro"] = macro;
  t["version"] = version;
  t["timestamp"] = timestamp.str();
  t["user"] = transaction.user;
  t["cid"] = c["uid"];
  data.transactions.push_back(t);

  // Iterate fields...
  Fields::iterator fi = commit.fields.begin();
  while(fi != commit.fields.end()) {
    std::string key = fi->first;
    std::string value = fi->second;

    if(ignore_fieldnames == false) {

      dbtable_t::iterator fni = select(data.fieldnames, "name", key);
      if(fni != data.fieldnames.end()) {
        dbrow_t f;
        f["transaction"] = data.trseq.currval();
        f["name"] = key;
        f["value"] = value;
        data.fields.push_back(f);
          
      }

    } else {

      dbrow_t f;
      f["transaction"] = data.trseq.currval();
      f["name"] = key;
      f["value"] = value;
      data.fields.push_back(f);

    }
    
    fi++;
  }
}

Values PracroDAOTest::getLatestValues(std::string sessionid,
                                      std::string patientid, Macro *macro,
                                      Fieldnames &fieldnames, time_t oldest)
{
  std::string macro_name = macro ? macro->name.c_str() : "(null)";
  DEBUG(db, "(%s, %s, <%u fieldnames>, %ld)\n",
        patientid.c_str(),
        macro_name.c_str(), (int)fieldnames.size(),
        oldest);
  Values values;

  Fieldnames::iterator fi = fieldnames.begin();
  while(fi != fieldnames.end()) {
    std::string fieldname = *fi;

    dbtable_t::iterator ci = data.commits.begin();
    while(ci != data.commits.end()) {
      dbrow_t &c = *ci;
    
      if(c["status"] == "committed" ||
         (c["status"] != "committed" && c["uid"] == sessionid)) {

        // Find matching transactions
        dbtable_t::iterator ti = data.transactions.begin();
        while(ti != data.transactions.end()) {
          dbrow_t &t = *ti;
          
          time_t timestamp = atol(t["timestamp"].c_str());
          
          if(t["cid"] == c["uid"] && timestamp >= oldest &&
             (t["macro"] == macro_name || macro == nullptr)) {
            
            std::string tid = t["uid"];
            
            // Find transaction values
            dbtable_t::iterator vi = data.fields.begin();
            while(vi != data.fields.end()) {
              dbrow_t &f = *vi;
              
              // Upon match, insert it into values
              if(f["transaction"] == tid && f["name"] == fieldname) {
                if(values.find(fieldname) == values.end() ||
                   values[fieldname].timestamp < timestamp) {
                  
                  values[fieldname].timestamp = timestamp;
                  values[fieldname].value = f["value"];
                  values[fieldname].source = "testdb";
                }
              }
              
              vi++;
            }
          }
          ti++;
        }
      }
      ci++;
    }
    fi++;
  }

  return values;
}

unsigned PracroDAOTest::nrOfCommits(std::string sessionid,
                                    std::string patientid,
                                    std::string macroname,
                                    time_t oldest)
{
  unsigned num = 0;

  dbtable_t::iterator ci = data.commits.begin();
  while(ci != data.commits.end()) {
    dbrow_t &c = *ci;
    time_t timestamp = atol(c["timestamp"].c_str());
    std::string cid = c["uid"];
    if(c["patientid"] == patientid && timestamp >= oldest) {

      dbtable_t::iterator ti = data.transactions.begin();
      while(ti != data.transactions.end()) {
        dbrow_t &t = *ti;
        if(t["cid"] == cid && t["macro"] == macroname) num++;
        ti++;
      }

    }
    ci++;
  }

  return num;
}

void PracroDAOTest::addFieldname(std::string name, std::string description)
{
  dbrow_t fieldname;
  fieldname["name"] = name;
  fieldname["description"] = description;
  char buf[256];
  sprintf(buf, "%lu", time(nullptr));
  fieldname["timestamp"] = buf;
  data.fieldnames.push_back(fieldname);
}

void PracroDAOTest::delFieldname(std::string name)
{
  dbtable_t::iterator i = select(data.fieldnames, "name", name);
  if(i != data.fieldnames.end()) data.fieldnames.erase(i);
}

std::vector<Fieldname> PracroDAOTest::getFieldnames()
{
  std::vector<Fieldname> fieldnames;

  dbtable_t::iterator i = data.fieldnames.begin();
  while(i != data.fieldnames.end()) {
    dbrow_t &row = *i;
    Fieldname fn;
    fn.name = row["name"];
    fn.description = row["description"];
    fn.timestamp = atoll(row["timestamp"].c_str());
    fieldnames.push_back(fn);
    i++;
  }

  return fieldnames;
}

bool PracroDAOTest::idle(std::string sessionid)
{
  dbtable_t::iterator i = select(data.commits, "uid", sessionid);
  if(i != data.commits.end()) {
    dbrow_t &commit = *i;
    return commit["status"] == "idle";
  }

  return false;
}

void PracroDAOTest::setIdle(std::string sessionid, bool idle)
{
  dbtable_t::iterator i = select(data.commits, "uid", sessionid);
  if(i != data.commits.end()) {
    dbrow_t &commit = *i;
    if(commit["status"] != "committed") {
      commit["status"] = idle?"idle":"active";
    }
  }
}

void PracroDAOTest::commit(std::string sessionid)
{
  dbtable_t::iterator i = select(data.commits, "uid", sessionid);
  if(i != data.commits.end()) {
    dbrow_t &commit = *i;
    if(commit["status"] != "committed") {
      commit["status"] = "committed";
    }
  }
}

void PracroDAOTest::nocommit(std::string sessionid)
{
  dbtable_t::iterator i = select(data.commits, "uid", sessionid);
  if(i != data.commits.end()) {
    dbrow_t &commit = *i;
    if(commit["status"] != "committed") {
      commit["status"] = "idle";
    }
  }
}

void PracroDAOTest::discard(std::string sessionid)
{
  dbtable_t::iterator i = select(data.commits, "uid", sessionid);
  if(i != data.commits.end()) {
    dbrow_t &commit = *i;
    if(commit["status"] != "committed") {
      data.commits.erase(i);
    }
  }
}

#ifdef TEST_PRACRODAOTEST
//deps: debug.cc log.cc configuration.cc exception.cc pracrodao.cc mutex.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
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

PracroDAOTest db;

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
  PracroDAOTest db(true);

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
  TEST_EQUAL_STR(vals["foo"].value, "hello", "Latest one only please");
  TEST_EQUAL_STR(vals["bar"].value, "world", "Latest one only please");
}


TEST_END;

#endif/*TEST_PRACRODAOTEST*/

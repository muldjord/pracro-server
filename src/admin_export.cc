/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            admin_export.cc
 *
 *  Fri Feb 11 11:43:13 CET 2011
 *  Copyright 2011 Bent Bisballe Nyeng
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
#include "admin_export.h"

#include <config.h>
#include <stdio.h>
#include <hugin.hpp>

#ifndef WITHOUT_DB

#include <stdlib.h>

#include "pgwork.h"

#include "fieldnamescanner.h"
#include "configuration.h"

#define SEP "\t"

static std::string escape(std::string &str)
{
  std::string out = "\"";
  std::string::iterator i = str.begin();
  while(i != str.end()) {
    if(*i == '\"') out += "''";
    else if(*i == '\n') out += "\342\220\244"; // N/L controlcharacter pictogram
    else if(*i == '\r') { }
    else out += *i;
    i++; 
  }
  out += "\"";
  return out;
}

class File {
public:
  File(fieldnames_t &f, Work &w)
    : work(w), fieldnames(f)
  {
    pos["id"] = 0;
    pos["patientid"] = 1;
    pos["time"] = 2;
    pos["template"] = 3;

    size_t idx = 4;
    fieldnames_t::iterator i = f.begin();
    while(i != f.end()) {
      pos[*i] = idx;
      idx++;
      i++;
    }

    output_header();
  }

  void output_header()
  {
    beginrow();
    addcell("id", "ID");
    addcell("patientid", "Patient");
    addcell("time", "Time");
    addcell("template", "Template");

    {
      result_t result =
        work.exec("SELECT DISTINCT ON(name) name, caption FROM fieldnames"
                  " WHERE extract='true';");
      result_t::const_iterator ri = result.begin();
      /*
      for(unsigned int r = 0; r < result.size(); r++) {
        tuple_t tuple = result.at(r); 
        std::string name = tuple.at(0).c_str();
        std::string caption = tuple.at(1).c_str();
        addcell(name, caption);
      }
      */
      while(ri != result.end()) {
        tuple_t tuple = *ri; 
        std::string name = tuple.at(0).c_str();
        std::string caption = tuple.at(1).c_str();
        if(caption.empty()) {
          caption = name;
        }
        addcell(name, caption);
        ri++;
      }

    }

    endrow();
  }

  void beginrow()
  {
    cells.clear();
    cells.insert(cells.begin(), pos.size(), "");
  }

  void addcell(std::string key, std::string value)
  {
    if(pos.find(key) == pos.end()) return;
    cells[pos[key]] = value;
  }

  void endrow()
  {
    std::vector<std::string>::iterator i = cells.begin();
    while(i != cells.end()) {
      result += escape(*i) + SEP;
      i++;
    }
    result += "\n";
  }

  std::string result;

private:
  Work &work;
  std::map<std::string, int> pos;
  std::vector<std::string> cells;
  std::string name;
  fieldnames_t &fieldnames;
};


static std::string do_export(Environment &env, std::string templ, bool *ok,
                             time_t from, time_t to)
{
  if(Conf::database_backend != "pgsql") {
    *ok = false;
    return "ERROR: Export only available for the pgsql database backend.\n";
  }

  std::string host = Conf::database_addr;
  std::string port = "";//Conf::database_port;
  std::string user = Conf::database_user;;
  std::string passwd = Conf::database_passwd;
  std::string dbname = "";//Conf::database_database;

  std::string cs;
  if(host.size()) cs += " host=" + host;
  if(port.size()) cs += " port=" + port;
  if(user.size()) cs += " user=" + user;
  if(passwd.size()) cs += " password=" + passwd;
  cs += " dbname=" + (dbname.size() ? dbname : "pracro");

  //  pqxx::connection conn(cs);
  PGconn *conn = PQconnectdb(cs.c_str());

  if(conn == nullptr || PQstatus(conn) == CONNECTION_BAD)	{
    ERR(db, "Postgresql init failed: %s\n", PQerrorMessage(conn));
    conn = nullptr;
    return "Postgresql init failed";
  }

  Work work(conn);

  std::set<std::string> filter;

  {
    result_t result =
      work.exec("SELECT DISTINCT name FROM fieldnames"
                " WHERE extract='true';");
    result_t::const_iterator ri = result.begin();
    /*
    for(unsigned int r = 0; r < result.size(); r++) {
      tuple_t tuple = result.at(r); 
      std::string name = tuple.at(0).c_str();
      filter.insert(name);
      //printf("filter: '%s'\n", name.c_str());
    }
    */
    while(ri != result.end()) {
      tuple_t tuple = *ri; 
      std::string name = tuple.at(0).c_str();
      filter.insert(name);
      //printf("filter: '%s'\n", name.c_str());
      ri++;
    }
  }

  templates_t t = scanfieldnames(env, filter);
  /*
  templates_t::iterator ti = t.begin();
  while(ti != t.end()) {
    printf("%s\n", ti->first.c_str());
    fieldnames_t::iterator fi = ti->second.begin();
    while(fi != ti->second.end()) {
      printf("\t%s\n", (*fi).c_str());
      fi++;
    }
    ti++;
  }
  */

  File file(t[templ], work);

  std::string tostr;
  std::string fromstr;
  {
    char buf[32];
    sprintf(buf, "%d", (int)from);
    fromstr = buf;
    sprintf(buf, "%d", (int)to);
    tostr = buf;
  }

  {
    std::string q = "SELECT * FROM commits WHERE template='"+templ+"'"
      " AND status='committed' AND timestamp>=" +fromstr+
      " AND timestamp<="+tostr+" ORDER BY patientid, timestamp;";

    DEBUG(export, "QUERY: %s\n", q.c_str());

    result_t result = work.exec(q);
    result_t::const_iterator ri = result.begin();
    //    for(unsigned int r = 0; r < result.size(); r++) {
    while(ri != result.end()) {
      tuple_t tuple = *ri; 
      std::string patientid = tuple.at(0).c_str();
      std::string templ = tuple.at(1).c_str();
      std::string version = tuple.at(2).c_str();
      std::string timestamp = tuple.at(3).c_str();
      std::string uid = tuple.at(4).c_str();
      //      std::string status = tuple.at(5).c_str();

      file.beginrow();
      file.addcell("id", uid);
      file.addcell("patientid", patientid);
      file.addcell("template", templ);
      time_t t = atol(timestamp.c_str());
      char tstr[20] = {'\0'};
      strftime(tstr, 20, "%d.%m.%Y %H:%M:%S", localtime(&t));
      std::string timestr = tstr;
      if(timestr[timestr.size() - 1] == '\n')
        timestr = timestr.substr(0, timestr.size() - 1);
      file.addcell("time", timestr.c_str());

      DEBUG(export, "%s %s %s\n",
            timestamp.c_str(), patientid.c_str(), templ.c_str());

      {
        result_t result =
          work.exec("SELECT f.name, f.value"
                    " FROM transactions t, fields f, fieldnames n"
                    " WHERE t.cid='"+uid+"' AND f.transaction=t.uid"
                    "  AND f.name=n.name AND n.extract='true'"
                    " ORDER BY t.timestamp;");
        result_t::const_iterator ri = result.begin();
        //for(unsigned int r = 0; r < result.size(); r++) {
        while(ri != result.end()) {
          tuple_t tuple = *ri; 
          std::string name = tuple.at(0).c_str();
          std::string value = tuple.at(1).c_str();

          file.addcell(name, value);
          ri++;
        }
      }

      file.endrow();
      ri++;
    }
  }
  
  *ok = true;

  // Make sure we stop using the connection before we close the connection.
  work.abort();
  PQfinish(conn);

  return file.result;
}

#endif/* WITHOUT_DB */

std::string admin_export(Environment &env, std::string templ, bool *ok,
                         time_t from, time_t to)
{
#ifndef WITHOUT_DB
  return do_export(env, templ, ok, from, to);
#else
  return "No database available";
#endif/* WITHOUT_DB */
}

#ifdef TEST_ADMIN_EXPORT
//deps: environment.cc connectionpool.cc mutex.cc semaphore.cc configuration.cc entitylist.cc artefact.cc database.cc pracrodao.cc pracrodaotest.cc pracrodaopgsql.cc templatelist.cc macrolist.cc debug.cc macroheaderparser.cc exception.cc log.cc saxparser.cc templateheaderparser.cc versionstr.cc inotify.cc session.cc sessionserialiser.cc journal.cc journal_uploadserver.cc journal_commit.cc xml_encode_decode.cc sessionparser.cc fieldnamescanner.cc util.cc macroparser.cc templateparser.cc courselist.cc luascript.cc sessionheaderparser.cc courseparser.cc luautil.cc
//cflags: -I.. -DWITHOUT_ARTEFACT $(PQXX_CFLAGS) $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(PQXX_LIBS) $(EXPAT_LIBS) $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_ADMIN_EXPORT*/

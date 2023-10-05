/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryhandlerpracro.cc
 *
 *  Thu Jan 15 11:35:34 CET 2009
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
#include "queryhandlerpracro.h"

#include <hugin.hpp>

#include <stdlib.h>

#include "configuration.h"

QueryHandlerPracro::QueryHandlerPracro(Database &_db, std::string cpr,
                                       std::string sessionid)
  : db(_db)
{
  this->cpr = cpr;
  this->sessionid = sessionid;
}

QueryResult QueryHandlerPracro::exec(Query &query)
{
  QueryResult result;
  result.timestamp = 0;
  result.source = "";

  std::string field = query.attributes["class"];
  Fieldnames fields;
  fields.push_back(field);

  time_t oldest;  
  if(query.attributes.find("ttl") != query.attributes.end()) {
    std::string ttl = query.attributes["ttl"];
    oldest = time(nullptr) - atol(ttl.c_str());
  } else {
    oldest = time(nullptr) - Conf::db_max_ttl;
  }

  Values values = db.getValues(cpr, fields, sessionid, oldest);

  if(values.find(field) != values.end()) {
    std::string value = values[field].value;
    time_t timestamp = values[field].timestamp;

    result.timestamp = timestamp;
    result.values[field] = value;
    result.source = "pracrodb";

    DEBUG(queryhandler,"%s => %s (%lu)\n",
          field.c_str(), value.c_str(), timestamp);
  }

  return result;
}

#ifdef TEST_QUERYHANDLERPRACRO
//deps: database.cc mutex.cc debug.cc log.cc pracrodao.cc pracrodaotest.cc pracrodaopgsql.cc configuration.cc
//cflags: -I.. $(PQXX_CXXFLAGS) $(PTHREAD_CFLAGS)
//libs: $(PQXX_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#include <time.h>

#define PATIENTID "1234567890"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  time_t now = time(nullptr);
  Database db("testdb", "", "", "", "", "");
  Macro macro;
  Fields fields;
  QueryHandlerPracro qh(db, PATIENTID);

  Query query;
  QueryResult res;

  fields["myfield"] = "myval";
  db.commitTransaction("testuser", PATIENTID, macro, fields, now - Conf::db_max_ttl - 1);

  query.attributes["class"] = "myfield";
  res = qh.exec(query);
  if(res.timestamp != 0) return 1;
  if(res.source != "") return 1;
  if(res.values.size() != 0) return 1;

  fields["myfield"] = "myval";
  db.commitTransaction("testuser", PATIENTID, macro, fields, now - 100);

  query.attributes["class"] = "myfield";
  query.attributes["ttl"] = "99";
  res = qh.exec(query);
  if(res.timestamp != 0) return 1;
  if(res.source != "") return 1;
  if(res.values.size() != 0) return 1;

  query.attributes["class"] = "nosuchfield";
  query.attributes["ttl"] = "10000";
  res = qh.exec(query);
  if(res.timestamp != 0) return 1;
  if(res.source != "") return 1;
  if(res.values.size() != 0) return 1;

  query.attributes["class"] = "myfield";
  query.attributes["ttl"] = "100";
  res = qh.exec(query);
  if(res.timestamp != now - 100) return 1;
  if(res.source != "pracrodb") return 1;
  if(res.values["myfield"] != "myval") return 1;
*/
TEST_END;

#endif/*TEST_QUERYHANDLERPRACRO*/

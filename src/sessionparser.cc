/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sessionparser.cc
 *
 *  Thu May 20 14:30:23 CEST 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#include "sessionparser.h"

#include <stdio.h>

#include <hugin.hpp>

SessionParser::SessionParser()
{
  done = false;
  totalbytes = 0;
  inresume = false;
  indatabase = false;
  invalue = false;
  inscript = false;
  inenv = false;
}

SessionParser::~SessionParser()
{
}

void SessionParser::characterData(std::string &data)
{
  if(inresume) {
    entries[entries.size()-1].resume += data;
  }

  if(inscript) {
    Entry &e = entries[entries.size() - 1];
    LUAOnCommit *oncommit = e.oncommit;
    std::pair<std::string, std::string> &val =
      oncommit->scripts[oncommit->scripts.size() - 1];
    val.first += data;
  }

  if(invalue) {
    Entry &e = entries[entries.size() - 1];
    LUAOnCommit *oncommit = e.oncommit;
    oncommit->values[valuename] += data;
  }

  if(inenv) {
    Entry &e = entries[entries.size() - 1];
    LUAOnCommit *oncommit = e.oncommit;
    oncommit->_env[envid] += data;
  }

  if(indatabase) {
    database += data;
  }
}

void SessionParser::startTag(std::string name, attributes_t &attr)
{
  DEBUG(sessionparser, "<%s>\n", name.c_str());

  if(name == "session") {
    patientid = attr["patientid"];
    sessionid = attr["id"];
    templ = attr["template"];
    status = attr["status"];
  }

  if(name == "journal") {
    //    patientid = attr["patientid"];
    //    userid = attr["userid"];
  }

  if(name == "database") {
    dbtype = attr["type"];
    indatabase = true;
  }

  if(name == "entry") {
    Entry e;
    e.index = atoi(attr["index"].c_str());
    e.macro = attr["macro"];
    e.user = attr["user"];
    e.oncommit = nullptr;
    entries.push_back(e);
  }

  if(name == "resume") {
    inresume = true;
  }

  if(name == "oncommit") {
    Entry &e = entries[entries.size() - 1];
    if(e.oncommit != nullptr) {
      ERR(sessionparser, "Multiple oncommit tags in journal!\n");
      return;
    }
    e.oncommit = new LUAOnCommit();
  }
  if(name == "envs") { }

  if(name == "env") {
    if(attr["id"] == "ENV_PATIENTID") envid = LUAScript::ENV_PATIENTID;
    else if(attr["id"] == "ENV_TEMPLATE") envid = LUAScript::ENV_TEMPLATE;
    else if(attr["id"] == "ENV_MACRO") envid = LUAScript::ENV_MACRO;
    else if(attr["id"] == "ENV_USER") envid = LUAScript::ENV_USER;
    else {
      // Unknown env id
      return;
    }
    inenv = true;
  }

  if(name == "values") { }

  if(name == "value") {
    valuename = attr["name"];
    invalue = true;
  }

  if(name == "scripts") {}

  if(name == "script") {
    Entry &e = entries[entries.size() - 1];
    LUAOnCommit *oncommit = e.oncommit;
    oncommit->addCode("", attr["name"]);
    inscript = true;
  }
}

void SessionParser::endTag(std::string name)
{
  if(name == "resume") {
    inresume = false;
  }
  if(name == "database") {
    indatabase = false;
  }
  if(name == "env") {
    inenv = false;
  }
  if(name == "value") {
    invalue = false;
  }
  if(name == "script") {
    inscript = false;
  }
}

void SessionParser::parseError(const char *buf, size_t len,
                               std::string error, int lineno)
{
  ERR(sessionnparser, "SessionParser error at line %d: %s\n",
      lineno, error.c_str());

  std::string xml;
  if(buf && len) xml.append(buf, len);

  ERR(sessionparser, "\tBuffer %u bytes: [%s]\n", (int)len, xml.c_str());

  fflush(stderr);

  throw std::exception();
}

#ifdef TEST_SESSIONPARSER
//deps: saxparser.cc debug.cc log.cc mutex.cc luascript.cc luautil.cc configuration.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_SESSIONPARSER*/

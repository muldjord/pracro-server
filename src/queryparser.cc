/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryparser.cc
 *
 *  Tue May  6 17:02:37 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
#include "queryparser.h"

#include <stdio.h>

QueryParser::QueryParser()
{
  this->timestamp = 0;

  p = 0;
  stack.push_back(&result);
}

void QueryParser::startTag(std::string name, attributes_t &attr)
{

  if(name == "results") {
    // What to do here!?
  }

  if(name == "result") {
    this->timestamp = atol(attr["timestamp"].c_str());

    QueryResult q;
    q.source = "pentominos";
    q.timestamp = this->timestamp;
    stack.back()->groups[attr["class"]] = q;
    stack.push_back(&stack.back()->groups[attr["class"]]);
  }

  if(name == "group") {
    QueryResult q;
    q.timestamp = this->timestamp;
    stack.back()->groups[attr["name"]] = q;
    stack.push_back(&stack.back()->groups[attr["name"]]);
  }

  if(name == "value") {
    stack.back()->values[attr["name"]] = utf8.decode(attr["value"]);
  }

}

void QueryParser::endTag(std::string name)
{
  if(name == "group" || name == "result") stack.pop_back();
}

void QueryParser::parseError(const char *buf, size_t len, std::string error, int lineno)
{
  fprintf(stderr, "QueryParser error at line %d: %s\n", lineno, error.c_str());
  fprintf(stderr, "\tBuffer %u bytes: [", (int)len);
  if(fwrite(buf, len, 1, stderr) != len) {}
  fprintf(stderr, "]\n");
  fflush(stderr);

  char *slineno;
  if(asprintf(&slineno, " at line %d\n", lineno) != -1) {
    throw Exception(error + slineno);
    free(slineno);
  }

}

#ifdef TEST_QUERYPARSER
//deps: saxparser.cc debug.cc log.cc utf8.cc exception.cc mutex.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#include <string.h>
/*
static char xml[] =
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<results>\n"
  "  <result class=\"testclass\" timestamp=\"1234567890\">\n"
  "    <group name=\"testgroup\">\n"
  "      <value name=\"testvalue\" value=\"42\"/>\n"
  "      <value name=\"anothertestvalue\" value=\"42\"/>\n"
  "    </group>\n"
  "    <group name=\"anothertestgroup\">\n"
  "      <value name=\"testvalue\" value=\"42\"/>\n"
  "      <value name=\"anothertestvalue\" value=\"42\"/>\n"
  "    </group>\n"
  "  </result>\n"
  "  <result class=\"anothertestclass\" timestamp=\"1234567890\">\n"
  "    <group name=\"testgroup\">\n"
  "      <value name=\"testvalue\" value=\"42\"/>\n"
  "      <value name=\"anothertestvalue\" value=\"42\"/>\n"
  "    </group>\n"
  "    <group name=\"anothertestgroup\">\n"
  "      <value name=\"testvalue\" value=\"42\"/>\n"
  "      <value name=\"anothertestvalue\" value=\"42\"/>\n"
  "    </group>\n"
  "  </result>\n"
  "</results>\n"
;

static char badxml[] =
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<results>\n"
  "</sulrets>\n"
;

static std::string loadresultstring(QueryResult &res, std::string group = "")
{
  std::string s;

  std::map< std::string, std::string >::iterator v = res.values.begin();
  while(v != res.values.end()) {
    s += group + (*v).first + " = \"" + (*v).second + "\"\n";
    v++;
  }

  std::map< std::string, QueryResult >::iterator g = res.groups.begin();
  while(g != res.groups.end()) {
    s += group + (*g).first + " = {}\n";
    s += loadresultstring((*g).second, group + (*g).first + ".");
    g++;
  }

  return s;
}
*/
TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  // Parse something
  {
    QueryParser parser;
    parser.parse(xml, strlen(xml));
    printf("%s\n", loadresultstring(parser.result).c_str());
  }

  // Parse something, and fail!
  try {
    QueryParser parser;
    parser.parse(badxml, strlen(badxml));
    printf("%s\n", loadresultstring(parser.result).c_str());
  } catch(Exception &e) {
    printf("ERROR: %s\n", e.what());
    goto weitergehen;
  }
  return 1;
 weitergehen:
*/
TEST_END;

#endif/*TEST_QUERYPARSER*/

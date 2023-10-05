/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luaquerymapper.cc
 *
 *  Mon May  5 15:43:52 CEST 2008
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
#include "luaquerymapper.h"

#include <sstream>
#include <string>

#include "luautil.h"

static void loadResult(lua_State *L, QueryResult &res,
                    std::vector<std::string> group = std::vector<std::string>())
{
  std::vector<std::string> groups;
  std::vector<std::string>::iterator gi = group.begin();
  while(gi != group.end()) {
    const char *name = gi->c_str();

    if(!Pracro::testField(L, groups, name)) {
      Pracro::createField(L, groups, name);
    }

    groups.push_back(name);
    gi++;
  }

  std::string s;
  std::stringstream timestamp; timestamp << res.timestamp;

  std::map< std::string, std::string >::iterator v = res.values.begin();
  while(v != res.values.end()) {

    const char *name = v->first.c_str();

    if(!Pracro::testField(L, group, name)) {
      Pracro::createField(L, group, name);
    }

    int top = lua_gettop(L);
    Pracro::getField(L, group, name);

    lua_pushstring(L, v->second.c_str());
    lua_setfield(L, -2, "value");

    lua_pushstring(L, timestamp.str().c_str());
    lua_setfield(L, -2, "timestamp");

    lua_pushstring(L, res.source.c_str());
    lua_setfield(L, -2, "source");

    while(lua_gettop(L) > top) lua_pop(L, 1);

    v++;
  }

  while(lua_gettop(L)) lua_pop(L, 1);

  //  printf("< %d\n", lua_gettop(L)); fflush(stdout);

  std::map< std::string, QueryResult >::iterator g = res.groups.begin();
  while(g != res.groups.end()) {
    std::vector<std::string> subgrp = group;
    subgrp.push_back(g->first);
    loadResult(L, g->second, subgrp);
    g++;
  }
}

LUAQueryMapper::LUAQueryMapper()
{
  clean_top = -1;

  L = luaL_newstate();
  if(L == nullptr) {
    error("Could not create LUA state.");
    return;
  }

  luaL_openlibs(L);

  clean_top = lua_gettop(L);
}

LUAQueryMapper::~LUAQueryMapper()
{
  if(L) lua_close(L);
}

static QueryResult splitgroups(QueryResult r)
{
  QueryResult result;

  result.timestamp = r.timestamp;
  result.source = r.source;

  std::map< std::string, QueryResult >::iterator gi = r.groups.begin();
  while(gi != r.groups.end()) {
    QueryResult child = splitgroups(gi->second);
    result.groups[gi->first] = child;
    gi++;
  }

  std::map< std::string, std::string >::iterator vi = r.values.begin();
  while(vi != r.values.end()) {
    std::string name = vi->first;
    
    // Also insert in table with name containing '.'.
    if(name.find(".") != std::string::npos) result.values[name] = vi->second;

    QueryResult *ncurrent = &result;
    while(name.find(".") != std::string::npos) {
      DEBUG(splitgroups, "value name: %s\n", name.c_str());
      QueryResult grp;
      grp.timestamp = ncurrent->timestamp;
      grp.source = ncurrent->source;
      std::string grpname = name.substr(0, name.find("."));
      ncurrent->groups[grpname] = grp;
      ncurrent = &(ncurrent->groups[grpname]);
      name = name.substr(name.find(".") + 1);
    }
    ncurrent->values[name] = vi->second;

    vi++;
  }
  
  return result;
}


void LUAQueryMapper::addQueryResult(QueryResult &result)
{
  // Check for '.' in names and further split up into groups.
  QueryResult splitted = splitgroups(result);

  loadResult(L, splitted);
  clean_top = lua_gettop(L);
}

Value LUAQueryMapper::map(const std::string &mapper)
{
  Value v;

  if(L == nullptr) {
    error("LUA state not initialized!");
    return v;
  }

  if(mapper == "") {
    error("Empty LUA mapper detected in " + mapper);
    return v;
  }

  DEBUG(querymapper, "Mapper: %s\n", mapper.c_str());

  // Load the mapper
  if(luaL_loadbuffer(L, mapper.c_str(), mapper.size(), "mapper")) {
    error(lua_tostring(L, lua_gettop(L)) + std::string(" in ") + mapper);
    return v;
  }

  // Run the loaded code
  if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
    error(lua_tostring(L, lua_gettop(L)) + std::string(" in ") + mapper);
    return v;
  }

  // Check if app messed up the stack.
  if(lua_gettop(L) != clean_top + 3) {
    error("Wrong number of return values (should be value, timestamp, source) in " + mapper);
    return v;
  }

  // Check if the types are right
  if(lua_isstring(L, lua_gettop(L)) == false) {
    error("Source is not a string in " + mapper);
    return v;
  }
  v.source = lua_tostring(L, lua_gettop(L));
  lua_pop(L, 1);

  // Check if the types are right
  if(lua_isnumber(L, lua_gettop(L)) == false) {
    error("Timestamp is not an integer in " + mapper);
    return v;
  }
  v.timestamp = lua_tointeger(L, lua_gettop(L));
  lua_pop(L, 1);

  // Check if the types are right
  if(lua_isstring(L, lua_gettop(L)) == false) {
    error("Value is not a string in " + mapper);
    return v;
  }
  v.value = lua_tostring(L, lua_gettop(L));
  lua_pop(L, 1);

  DEBUG(querymapper, "Result: value=%s, src=%s, time=%d\n",
        v.value.c_str(), v.source.c_str(), (int)v.timestamp);

  return v;
}

void LUAQueryMapper::error(std::string message)
{
  if(clean_top != -1) lua_pop(L, lua_gettop(L) - clean_top); // Clean up stack
  throw Exception("ERROR in LUAQueryMapper: " + message);
}

std::string LUAQueryMapper::automap(const std::string &name)
{
  std::string group;
  std::string groupcheck = "if(";

  for(size_t i = 0; i < name.length(); i++) {
    group += name[i];
    if(name[i] == '.') groupcheck += " and " + group;
    else groupcheck += name[i];
  }
  groupcheck += " and " + name + ".value and " + name + ".timestamp and " + name + ".source";
  groupcheck += ")\n";

  std::string automapstring =
    "-- Returning 0, 0 invalidates the result\n"
    "value = 0\n"
    "timestamp = 0\n"
    "source = 0\n"
    "\n"
    + groupcheck + 
    "then\n"
    "  value = " + name + ".value\n"
    "  timestamp = " + name + ".timestamp\n"
    "  source = " + name + ".source\n"
    "end\n"
    "return value, timestamp, source\n";

  DEBUG(widget, "Automap:\n%s\n", automapstring.c_str());

  return automapstring;
}

#ifdef TEST_LUAQUERYMAPPER
//deps: exception.cc debug.cc log.cc mutex.cc luascript.cc luautil.cc saxparser.cc configuration.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS) $(EXPAT_LIBS)
#include <test.h>

TEST_BEGIN;

time_t now = time(nullptr);

{
  LUAQueryMapper mapper;

  QueryResult res;
  res.groups["test"].timestamp = now;
  res.groups["test"].source = "test app";
  res.groups["test"].values["somevalue"] = "hello world";
  res.groups["test"].values["pi"] = "3.14";
  res.groups["test"].groups["subtest"].source = "test app 2";
  res.groups["test"].groups["subtest"].timestamp = now + 1;
  res.groups["test"].groups["subtest"].values["somevalue"] = "hello world!";
  res.groups["test"].groups["subtest"].groups["subsubtest"].source = "test app 3";
  res.groups["test"].groups["subtest"].groups["subsubtest"].timestamp = now + 2;
  res.groups["test"].groups["subtest"].groups["subsubtest"].values["somevalue"] = "hello world!!";

  mapper.addQueryResult(res);

  QueryResult res2;
  res2.groups["test"].groups["subtest"].source = "src";
  res2.groups["test"].groups["subtest"].timestamp = now + 3;
  res2.groups["test"].groups["subtest"].values["val"] = "gnaf";
  res2.groups["test"].groups["subgrp"].source = "src2";
  res2.groups["test"].groups["subgrp"].timestamp = now + 4;
  res2.groups["test"].groups["subgrp"].values["val2"] = "gnaf2";

  mapper.addQueryResult(res2);

  // Test simple value forwarding with nesting
  std::string luamap = "return test.subtest.subsubtest.somevalue.value, test.subtest.subsubtest.somevalue.timestamp, test.subtest.subsubtest.somevalue.source";

  Value value = mapper.map(luamap);

  TEST_EQUAL_STR(value.value, "hello world!!", "Test value");
  TEST_EQUAL_INT(value.timestamp, now + 2, "Test timestamp");
  TEST_EQUAL_STR(value.source, "test app 3", "Test source");

  luamap = "return test.subtest.somevalue.value, test.subtest.somevalue.timestamp, test.subtest.somevalue.source";

  value = mapper.map(luamap);

  TEST_EQUAL_STR(value.value, "hello world!", "Test value");
  TEST_EQUAL_INT(value.timestamp, now + 1, "Test timestamp");
  TEST_EQUAL_STR(value.source, "test app 2", "Test source");

  luamap = "return test.somevalue.value, test.somevalue.timestamp, test.somevalue.source";

  value = mapper.map(luamap);

  TEST_EQUAL_STR(value.value, "hello world", "Test value");
  TEST_EQUAL_INT(value.timestamp, now, "Test timestamp");
  TEST_EQUAL_STR(value.source, "test app", "Test source");

  luamap = "return test.pi.value, test.pi.timestamp, test.pi.source";

  value = mapper.map(luamap);

  TEST_EQUAL_STR(value.value, "3.14", "Test value");
  TEST_EQUAL_INT(value.timestamp, now, "Test timestamp");
  TEST_EQUAL_STR(value.source, "test app", "Test source");

  luamap = "return test.subtest.val.value, test.subtest.val.timestamp, test.subtest.val.source";

  value = mapper.map(luamap);

  TEST_EQUAL_STR(value.value, "gnaf", "Test value");
  TEST_EQUAL_INT(value.timestamp, now + 3, "Test timestamp");
  TEST_EQUAL_STR(value.source, "src", "Test source");

  luamap = "return test.subgrp.val2.value, test.subgrp.val2.timestamp, test.subgrp.val2.source";

  value = mapper.map(luamap);

  TEST_EQUAL_STR(value.value, "gnaf2", "Test value");
  TEST_EQUAL_INT(value.timestamp, now + 4, "Test timestamp");
  TEST_EQUAL_STR(value.source, "src2", "Test source");
}

QueryResult res;
res.groups["test"].timestamp = now;
res.groups["test"].source = "test app";
res.groups["test"].values["somevalue"] = "hello world";
res.groups["test"].values["pi"] = "3.1416";

LUAQueryMapper mapper;
mapper.addQueryResult(res);

// Test simple value forwarding
std::string luamap = "return test.somevalue.value, test.somevalue.timestamp, test.somevalue.source";

Value value = mapper.map(luamap);

TEST_EQUAL_STR(value.value, "hello world", "Test value");
TEST_EQUAL_INT(value.timestamp, now, "Test timestamp");
TEST_EQUAL_STR(value.source, "test app", "Test source");

// Do some calculations
luamap = "return 2 * tonumber(test.pi.value), test.pi.timestamp, test.pi.source";
value = mapper.map(luamap);

TEST_EQUAL_STR(value.value, "6.2832", "Test value");
TEST_EQUAL_INT(value.timestamp, now, "Test timestamp");
TEST_EQUAL_STR(value.source, "test app", "Test source");

// Attempt to access nonexisting value (should throw an exception)
luamap = "return test.somevalue2.value, test.somevalue2.timestamp, test.somevalue2.source";
TEST_EXCEPTION(mapper.map(luamap), Exception, "Throw exception");

// Attempt to access nonexisting group (should throw an exception)
luamap = "return test2.somevalue.value, test2.somevalue.timestamp, test2.somevalue.source";
TEST_EXCEPTION(mapper.map(luamap), Exception, "Throw exception");
  
// Switch order of return vars (should throw an exception)
luamap = "return test.somevalue.source, test.somevalue.value, test.somevalue.timestamp";
TEST_EXCEPTION(mapper.map(luamap), Exception, "Throw exception");

// Syntax error (should throw an exception)
luamap = "this(is{] not() - a != legal lua program!]";
TEST_EXCEPTION(mapper.map(luamap), Exception, "Throw exception");

// And finally test if we haven't broken enything while being hostile to the lua engine...
luamap = "return test.somevalue.value, test.somevalue.timestamp, test.somevalue.source";
TEST_NOEXCEPTION(mapper.map(luamap), "Throw no exception");
TEST_EQUAL_STR(value.value, "6.2832", "Test value");
TEST_EQUAL_INT(value.timestamp, now, "Test timestamp");
TEST_EQUAL_STR(value.source, "test app", "Test source");

TEST_END;

#endif/*TEST_LUAQUERYMAPPER*/

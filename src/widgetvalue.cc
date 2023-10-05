/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            widgetvalue.cc
 *
 *  Fri Jul  2 11:40:12 CEST 2010
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
#include "widgetvalue.h"

#include <hugin.hpp>
#include "configuration.h"

static bool getMapValue(Value &value,
                        maps_t &maps,
                        std::string map,
                        LUAQueryMapper &mapper)
{
  std::string luamap;

  maps_t::iterator li = maps.begin();
  while(li != maps.end()) {
    Map &_map = *li;
    if(_map.attributes.find("name") != _map.attributes.end() &&
       _map.attributes["name"] == map) {
      if(_map.attributes.find("lua") != _map.attributes.end())
        luamap = _map.attributes["lua"];
    }
    li++;
  }

  // Check to see if we should automap
  if(luamap == "") {
    luamap = mapper.automap(map);
  }
    
    //    printf("LUAMAP: %s\n", luamap.c_str()); fflush(stdout);

  if(luamap != "") {
    value = mapper.map(luamap);

    // Value too old?
    if(value.timestamp < time(nullptr) - Conf::pentominos_max_ttl) return false;

    DEBUG(prefill, "map: (%s, %d)\n",
          value.value.c_str(),
          (int)value.timestamp);
    
  }

  return true;
}

static bool getDBValue(Value &value,
                       attr_t &attr,
                       Values &values)
{
  if(attr.find("name") == attr.end()) return false;

  // Check if there is a previously stored value in the db...
  if(values.find(attr["name"]) == values.end()) return false;

  value = values[attr["name"]];

  value.source = "pracro";

  return true;
}

bool getValue(Value &value,
              attr_t &attr,
              maps_t &maps,
              LUAQueryMapper &mapper,
              Values &values)
{
  std::map<time_t, Value> prio;

  // Value of value-tag.
  if(attr.find("value") != attr.end()) {
    Value v;
    v.timestamp = 0;
    v.source = "";
    v.value = attr["value"];
    prio[v.timestamp] = v;
  }

  // Value from query map
  if(attr.find("map") != attr.end()) {
    Value v;
    if(getMapValue(v, maps, attr["map"], mapper)) {
      prio[v.timestamp] = v;
    }
  }

  // Value from database
  {
    Value v;
    if(getDBValue(v, attr, values)) {
      prio[v.timestamp] = v;
    }
  }

  if(prio.size() != 0) {
    value = prio.rbegin()->second;
  }

  std::map<time_t, Value>::iterator i = prio.begin();
  while(i != prio.end()) {
    DEBUG(prefill, "% 11ld - \"%s\" (src: '%s')\n",
          i->second.timestamp,
          i->second.value.c_str(),
          i->second.source.c_str());
    i++;
  }

  return prio.size() != 0;
}


#ifdef TEST_WIDGETVALUE
//deps: debug.cc exception.cc log.cc configuration.cc luaquerymapper.cc mutex.cc luautil.cc
//cflags: -I.. $(LUA_CXXFLAGS) $(PTHREAD_CFLAGS)
//libs: $(LUA_LIBS) $(PTHREAD_LIBS)
#include "test.h"

TEST_BEGIN;

debug_parse("+all");

time_t now = time(nullptr);

{
  Value value;
  attr_t attr;
  maps_t maps;
  LUAQueryMapper mapper;
  Values values;
  TEST_FALSE(getValue(value, attr, maps, mapper, values), "Empty values");
}

{
  Value value;
  attr_t attr;
  attr["value"] = "hello";
  maps_t maps;
  LUAQueryMapper mapper;
  Values values;
  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(attr["value"], value.value, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, 0, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, "", "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";

  maps_t maps;
  LUAQueryMapper mapper;

  Values values;
  Value v;
  v.value = "world";
  v.source = "pracro";
  v.timestamp = now - (Conf::db_max_ttl * 2);
  values["foo"] = v;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(attr["value"], value.value, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, 0, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, "", "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";

  maps_t maps;
  LUAQueryMapper mapper;

  Values values;
  Value v;
  v.value = "world";
  v.source = "pracro";
  v.timestamp = now;
  values["foo"] = v;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(v.value, value.value, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, v.timestamp, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, v.source, "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;
  Conf::pentominos_max_ttl = 500;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";
  attr["map"] = "bar";

  maps_t maps;
  Map m;
  char tbuf[32]; sprintf(tbuf, "%ld", now);
  std::string val = "le valu";
  m.attributes["name"] = "bar";
  m.attributes["lua"] = "return '"+val+"', "+tbuf+", 'artefact'";
  maps.push_back(m);
  LUAQueryMapper mapper;

  Values values;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(value.value, val, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, now, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, "artefact", "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;
  Conf::pentominos_max_ttl = 500;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";
  attr["map"] = "bar";

  maps_t maps;
  Map m;
  char tbuf[32]; sprintf(tbuf, "%ld", now - (Conf::pentominos_max_ttl * 2));
  std::string val = "le valu";
  m.attributes["name"] = "bar";
  m.attributes["lua"] = "return '"+val+"', "+tbuf+", 'artefact'";
  maps.push_back(m);
  LUAQueryMapper mapper;

  Values values;
  Value v;
  v.value = "world";
  v.source = "pracro";
  v.timestamp = now;
  values["foo"] = v;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(v.value, value.value, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, v.timestamp, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, v.source, "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;
  Conf::pentominos_max_ttl = 500;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";
  attr["map"] = "bar";

  maps_t maps;
  Map m;
  char tbuf[32]; sprintf(tbuf, "%ld", now);
  std::string val = "le valu";
  m.attributes["name"] = "bar";
  m.attributes["lua"] = "return '"+val+"', "+tbuf+", 'artefact'";
  maps.push_back(m);
  LUAQueryMapper mapper;

  Values values;
  Value v;
  v.value = "world";
  v.source = "pracro";
  v.timestamp = now - 1;
  values["foo"] = v;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(value.value, val, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, now, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, "artefact", "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;
  Conf::pentominos_max_ttl = 500;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";
  attr["map"] = "bar";

  maps_t maps;
  Map m;
  char tbuf[32]; sprintf(tbuf, "%ld", now - 1);
  std::string val = "le valu";
  m.attributes["name"] = "bar";
  m.attributes["lua"] = "return '"+val+"', "+tbuf+", 'artefact'";
  maps.push_back(m);
  LUAQueryMapper mapper;

  Values values;
  Value v;
  v.value = "world";
  v.source = "pracro";
  v.timestamp = now    ;
  values["foo"] = v;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(value.value, v.value, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, v.timestamp, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, v.source, "Got the right source?");
}

{
  Conf::db_max_ttl = 1000;
  Conf::pentominos_max_ttl = 500;

  Value value;

  attr_t attr;
  attr["name"] = "foo";
  attr["value"] = "hello";
  attr["map"] = "bar";

  maps_t maps;
  Map m;
  char tbuf[32]; sprintf(tbuf, "%ld", now - 1);
  std::string val = "le valu";
  m.attributes["name"] = "bar";
  m.attributes["lua"] = "return '"+val+"', "+tbuf+", 'artefact'";
  maps.push_back(m);
  LUAQueryMapper mapper;

  Values values;
  Value v;
  v.value = "world";
  v.source = "pracro";
  v.timestamp = now    ;
  values["foo"] = v;

  TEST_TRUE(getValue(value, attr, maps, mapper, values), "Got value?");
  TEST_EQUAL_STR(value.value, v.value, "Got the right value?");
  TEST_EQUAL_INT(value.timestamp, v.timestamp, "Got the right timestamp?");
  TEST_EQUAL_STR(value.source, v.source, "Got the right source?");
}

TEST_END;

#endif/*TEST_WIDGETVALUE*/

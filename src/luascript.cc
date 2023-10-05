/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            luascript.cc
 *
 *  Tue Jan 10 14:43:39 CET 2012
 *  Copyright 2012 Bent Bisballe Nyeng
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
#include "luascript.h"

#include "configuration.h"
#include <hugin.hpp>

#include "luautil.h"

#define GLOBAL_POINTER "_pracroGlobalLUAObjectPointerThisShouldBeANameThatIsNotAccidentallyOverwritten"

static int _debug(lua_State *L)
{
  Pracro::checkParameters(L,
                          Pracro::T_STRING,
                          Pracro::T_END);

  std::string msg = lua_tostring(L, lua_gettop(L));

  DEBUG(luascript, "%s\n", msg.c_str());

  return 0;
}

static int _value(lua_State *L)
{
  Pracro::checkParameters(L,
                          Pracro::T_STRING,
                          Pracro::T_END);

  std::string name = lua_tostring(L, lua_gettop(L));

  lua_getglobal(L, GLOBAL_POINTER);
  LUAScript *lua = (LUAScript*)lua_touserdata(L, lua_gettop(L));

  if(!lua) {
    lua_pushstring(L, "No LUA pointer!");
    lua_error(L);
    return 1;
  }

  if(lua->hasValue(name)) {
    lua_pushstring(L, lua->value(name).c_str());
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int _patientid(lua_State *L)
{
  Pracro::checkParameters(L, Pracro::T_END);

  lua_getglobal(L, GLOBAL_POINTER);
  LUAScript *lua = (LUAScript*)lua_touserdata(L, lua_gettop(L));

  if(!lua) {
    lua_pushstring(L, "No LUA pointer!");
    lua_error(L);
    return 1;
  }

  lua_pushstring(L, lua->env(LUAScript::ENV_PATIENTID).c_str());

  return 1;
}

static int _template(lua_State *L)
{
  Pracro::checkParameters(L, Pracro::T_END);

  lua_getglobal(L, GLOBAL_POINTER);
  LUAScript *lua = (LUAScript*)lua_touserdata(L, lua_gettop(L));

  if(!lua) {
    lua_pushstring(L, "No LUA pointer!");
    lua_error(L);
    return 1;
  }

  lua_pushstring(L, lua->env(LUAScript::ENV_TEMPLATE).c_str());

  return 1;
}

static int _macro(lua_State *L)
{
  Pracro::checkParameters(L, Pracro::T_END);

  lua_getglobal(L, GLOBAL_POINTER);
  LUAScript *lua = (LUAScript*)lua_touserdata(L, lua_gettop(L));

  if(!lua) {
    lua_pushstring(L, "No LUA pointer!");
    lua_error(L);
    return 1;
  }

  lua_pushstring(L, lua->env(LUAScript::ENV_MACRO).c_str());

  return 1;
}

static int _user(lua_State *L)
{
  Pracro::checkParameters(L, Pracro::T_END);

  lua_getglobal(L, GLOBAL_POINTER);
  LUAScript *lua = (LUAScript*)lua_touserdata(L, lua_gettop(L));

  if(!lua) {
    lua_pushstring(L, "No LUA pointer!");
    lua_error(L);
    return 1;
  }

  lua_pushstring(L, lua->env(LUAScript::ENV_USER).c_str());

  return 1;
}

LUAScript::LUAScript()
{
  L = nullptr;
}

LUAScript::~LUAScript()
{
  if(L) {
    lua_close(L);
    L = nullptr;
  }
}

void LUAScript::init()
{
  if(L) return;

  L = luaL_newstate();
  if(L == nullptr) {
    ERR(luascript, "Could not create LUA state.\n");
    throw Exception("Could not create LUA state.");
  }

  luaL_openlibs(L);               

  lua_pushlightuserdata(L, this); // Push the pointer to 'this' instance
  lua_setglobal(L, GLOBAL_POINTER); // Assign it to a global lua var.

  lua_register(L, "value", _value);
  lua_register(L, "patientid", _patientid);
  lua_register(L, "template", _template);
  lua_register(L, "macro", _macro);
  lua_register(L, "user", _user);
  lua_register(L, "debug", _debug);

}

void LUAScript::addFile(std::string src)
{
  std::string file =
    Conf::xml_basedir + "/include/" + src;
  FILE *fp = fopen(file.c_str(), "r");
  if(fp) {
    char buf[64];
    size_t sz;
    std::string inc;
    while((sz = fread(buf, 1, sizeof(buf), fp)) != 0) {
      inc.append(buf, sz);
    }
    fclose(fp);
    addCode(inc, file);
  }
}

void LUAScript::addCode(std::string c, std::string name)
{
  scripts.push_back(std::make_pair(c, name));
}

void LUAScript::addValue(std::string name, const std::string &value)
{
  values[name] = value;
}

void LUAScript::addScripts(std::vector< Script > &scripts)
{
  std::vector< Script >::iterator spi = scripts.begin();
  while(spi != scripts.end()) {
    if(spi->attributes.find("src") != spi->attributes.end()) {
      std::string src = spi->attributes["src"];
      addFile(src);
    } else {
      addCode(spi->code);
    }
    spi++;
  }
}

void LUAScript::run()
{
  try {
    init();
  } catch(Exception &e) {
    throw Exception(e.msg);
  }

  if(L == nullptr) {
    ERR(luascript, "LUA state not initialized!");
    return;
  }

  top = lua_gettop(L);

  std::vector<std::pair<std::string, std::string> >::iterator i =
    scripts.begin();
  while(i != scripts.end()) {
    std::string program = i->first;
    std::string codename = name();
    if(i->second != "") codename += ": " + i->second;

    DEBUG(luascript, "Running %s: %s\n", codename.c_str(), program.c_str());

    if(luaL_loadbuffer(L, program.c_str(), program.size(), codename.c_str())) {
      ERR(luascript, "loadbuffer: %s\n", lua_tostring(L, lua_gettop(L)));
      throw Exception(lua_tostring(L, lua_gettop(L)));
    }
    
    // Run the loaded code
    if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
      ERR(luascript, "pcall: %s\n" , lua_tostring(L, lua_gettop(L)));
      throw Exception(lua_tostring(L, lua_gettop(L)));
    }

    i++;
  }
}

std::string LUAScript::resultString()
{
  if(top != lua_gettop(L) - 1) {
    ERR(luascript, "Program did not return a single value.\n");
    throw Exception("Program did not return a single value.");
  }

  if(lua_isstring(L, lua_gettop(L)) == false) {
    ERR(luascript, "Program did not return a string value.\n");
    throw Exception("Program did not return a string value.");
  }

  std::string res = lua_tostring(L, lua_gettop(L));
  lua_pop(L, 1);

  return res;
}

bool LUAScript::hasValue(std::string name)
{
  return values.find(name) != values.end();
}

std::string LUAScript::value(std::string name)
{
  if(values.find(name) != values.end()) return values[name];
  return "";
}

std::string LUAScript::env(LUAScript::env_t id)
{
  if(_env.find(id) == _env.end()) return "";
  return _env[id];
}

void LUAScript::setEnv(LUAScript::env_t id, std::string value)
{
  _env[id] = value;
}

#ifdef TEST_LUASCRIPT
//deps: exception.cc debug.cc log.cc mutex.cc luautil.cc saxparser.cc configuration.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS) $(EXPAT_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_LUASCRIPT*/

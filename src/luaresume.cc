/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            lua.cc
 *
 *  Thu May 29 16:30:50 CEST 2008
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
#include "luaresume.h"

#include <lua.hpp>
#include <lauxlib.h>

#include "luautil.h"

#include <hugin.hpp>

#include <stdio.h>

LUAResume::LUAResume(Transaction &t, Commit &c) : LUAScript()
{
  setEnv(LUAScript::ENV_PATIENTID, t.patientid);
  setEnv(LUAScript::ENV_TEMPLATE, c.templ);
  setEnv(LUAScript::ENV_MACRO, c.macro);
  setEnv(LUAScript::ENV_USER, t.user);

  std::map<std::string, std::string>::iterator i = c.fields.begin();
  while(i != c.fields.end()) {
    addValue(i->first, i->second);
    i++;
  }
}

#ifdef TEST_LUARESUME
//deps: exception.cc debug.cc log.cc mutex.cc luascript.cc luautil.cc saxparser.cc configuration.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS) $(EXPAT_LIBS)
#include <test.h>

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_LUARESUME*/

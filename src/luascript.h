/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            luascript.h
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
#ifndef __PRACRO_LUASCRIPT_H__
#define __PRACRO_LUASCRIPT_H__

#include <lua.hpp>
#include <lauxlib.h>

#include <string>
#include <map>
#include <vector>

#include "template.h"

class LUAScript {
  friend class SessionSerialiser;
  friend class SessionParser;
public:
  typedef enum {
    ENV_PATIENTID,
    ENV_TEMPLATE,
    ENV_MACRO,
    ENV_USER
  } env_t;

  class Exception {
  public:
    Exception(std::string m) : msg(m) {}
    std::string msg;
  };

  LUAScript();
  virtual ~LUAScript();

  virtual const char *name() { return ""; }

  void init();

  void addFile(std::string file);
  void addCode(std::string code, std::string codename = "");
  void addScripts(std::vector< Script > &scripts);

  void addValue(std::string name, const std::string &value);

  void run();

  bool hasValue(std::string name);
  std::string value(std::string value);

  std::string env(env_t id);
  void setEnv(env_t id, std::string value);

  std::string resultString();

protected:
  lua_State *L;

  std::map<env_t, std::string> _env;

private:
  std::vector<std::pair<std::string, std::string> > scripts;
  std::map<std::string, std::string> values;

  int top;
};


#endif/*__PRACRO_LUASCRIPT_H__*/

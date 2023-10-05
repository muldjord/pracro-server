/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luautil.h
 *
 *  Fri Apr 13 14:38:53 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
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
#ifndef __PRACRO_LUAUTIL_H__
#define __PRACRO_LUAUTIL_H__

#include <lua.hpp>
#include <lauxlib.h>

#include <string>
#include <vector>

namespace Pracro {

  /**
   * Set a global pointer that can be reaced from the cFunctions at a later time,
   * using the getGlobal function.
   * @param L The lua_State (active program) from which to get the pointer.
   * @param name The symbolic name in which to store the pointer.
   * @param p The pointer to set.
   */
  void setGlobal(lua_State *L, std::string name, void *p);
  
  /**
   * Get a global pointer set by the setGlobal function.
   * @param L The lua_State (active program) in which to set the pointer.
   * @param name The symbolic name in which the pointer is stored.
   * @return The pointer.
   */
  void *getGlobal(lua_State *L, std::string name);
  
  /**
   * Call a function in a lua program.
   * @param L The lua_State (active program) in which to set the function resides.
   * @param function The name of the function to be called.
   */
  void call(lua_State *L, std::string function, int numargs = 0);

  typedef enum {
    T_STRING,
    T_NUMBER,
    T_BOOLEAN,
    T_END
  } types_t;
  
  /**
   * Check parameter types and number.
   * @param L The lua_State (active program) in which to set the function resides.
   * @param types The type list (c-vector), describing the required types
   * on the stack. The last type must be a terminating T_END.
   * @return 0 on success. On error a long jump is made through lua_error, thus
   * the function never returns.
   */
  //  int checkParameters(lua_State *L, types_t types[]);
  int checkParameters(lua_State *L, ...);

  bool hasField(lua_State *L, int i, std::string name);

  bool testField(lua_State *L, std::vector<std::string> groups,
                 std::string field);

  bool getField(lua_State *L, std::vector<std::string> groups,
                std::string field);

  bool createField(lua_State *L, std::vector<std::string> groups,
                   std::string field);

  void stack(lua_State* l, const char *fmt, ...);
};

#endif/*__PRACRO_LUAUTIL_H__*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luaquerymapper.h
 *
 *  Mon May  5 15:43:51 CEST 2008
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
#ifndef __PRACRO_LUAQUERYMAPPER_H__
#define __PRACRO_LUAQUERYMAPPER_H__

#include "queryresult.h"

#include <lua.hpp>
#include <lauxlib.h>

// For class Value
#include "dbtypes.h"

#include "exception.h"

/**
 * The LUAQueryMapper class takes the result of an external data query and
 * applies the associated map.
 */
class LUAQueryMapper {
public:
  /**
   * Constructor.
   * Initialises the LUA library, and opens a LUA instance.
   * Throws an exception if any of these things fail.
   */
  LUAQueryMapper();
  ~LUAQueryMapper();

  /**
   * Applies the mapping program to the result-namespace, and returns the
   * resulting value. The map must return 3 values; value (a string), timestamp
   * (an integer) and the source (string).
   * If the program fails to load, fails to run, returns wrong number of values,
   * or the wrong types, it will thorw an exception.
   * @param mapper A std::string containing the mapping program.
   * @return A Value object containing the three result values.
   */
  Value map(const std::string &mapper);
 
  /**
   * Add a result to be included in the mapper namespace.
   * If in some way the generated values fail (illegal names for example) an exception
   * will be thrown.
   * @param result The QueryResult object containing the values to add to the
   * namespace.
   */
  void addQueryResult(QueryResult &result);

  std::string automap(const std::string &name);

private:
  void error(std::string message);

  lua_State *L;
  int clean_top;
};


#endif/*__PRACRO_LUAQUERYMAPPER_H__*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryparser.h
 *
 *  Tue May  6 17:02:36 CEST 2008
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
#ifndef __PRACRO_QUERYPARSER_H__
#define __PRACRO_QUERYPARSER_H__

#include "saxparser.h"

#include <time.h>
#include <vector>

#include "queryresult.h"
#include "utf8.h"
#include "exception.h"

/**
 * This class parses xml entities into a QueryResult structure.
 * Call the parent (SAXParser) method parse in order to actually parse something.
 * If the parser fails (syntax error) it will throw an Exception.
 * @see QueryResult result, in order to get the parsed data.
 */
class QueryParser : public SAXParser {
public:
  /**
   * Constructor.
   */
  QueryParser();

  /**
   * The object will contain the result when the parsing is done.
   */
  QueryResult result;

  /**
   * Private parser callback.
   */
  void startTag(std::string name, attributes_t &attr);

  /**
   * Private parser callback.
   */
  void endTag(std::string name);

  /**
   * Private parser callback.
   */
  void parseError(const char *buf, size_t len, std::string error, int lineno);

private:
  // For read
  int p;
  std::string document;
  time_t timestamp;
  std::vector< QueryResult * > stack;
  UTF8 utf8;
};

#endif/*__PRACRO_QUERYPARSER_H__*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            courseparser.h
 *
 *  Tue Jun 28 10:52:41 CEST 2011
 *  Copyright 2011 Bent Bisballe Nyeng
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
#ifndef __PRACRO_COURSEPARSER_H__
#define __PRACRO_COURSEPARSER_H__

#include "saxparser.h"
#include "template.h"

typedef enum {
  C_UNDEFINED,
  C_TEMPLATE,
  C_COURSE
} CourseParserState;

class CourseParser : public SAXParser {
public:
  CourseParser(std::string course);
  ~CourseParser();

  void characterData(std::string &data);
  void startTag(std::string name, attributes_t &attr);
  void endTag(std::string name);
  void parseError(const char *buf, size_t len, std::string error, int lineno);

  Course *getCourse();

protected:
  int readData(char *data, size_t size);

private:
  int fd;

  std::string file;

  // Parser state data
  CourseParserState state;
  Course *c;

  // Error callback function.
  void error(const char* fmt, ...);
};

#endif/*__PRACRO_COURSEPARSER_H__*/

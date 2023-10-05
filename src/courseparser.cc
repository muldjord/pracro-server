/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            courseparser.cc
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
#include "courseparser.h"

// For assert
#include <assert.h>

// For open and friends
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// For vprintf and friends
#include <stdarg.h>

#include <errno.h>
#include <string.h>

#include <stdio.h>

#include "configuration.h"
#include <hugin.hpp>
#include "exception.h"

void CourseParser::error(const char* fmt, ...)
{
  va_list argp;
  char *p;
  va_start(argp, fmt);
  vasprintf(&p, fmt, argp);
  va_end(argp);
  ERR(course, "Error in CourseParser: %s\n", p);
  throw Exception(std::string("Error in CourseParser: ") + p);
  free(p);
}

CourseParser::CourseParser(std::string coursefile)
{
  state = C_UNDEFINED;
  c = new Course();

  file = coursefile;

  DEBUG(course, "Using course file: %s\n", file.c_str());

  fd = open(file.c_str(), O_RDONLY);
  if(fd == -1) error("Could not open file %s. Errno was %i, %s", file.c_str(), errno, strerror(errno));
}

CourseParser::~CourseParser()
{
  if(fd != -1) close(fd);
  delete c;
}

void CourseParser::characterData(std::string &data)
{
}

void CourseParser::startTag(std::string name, attributes_t &attr)
{
  if(name == "course") {
    if(state != C_UNDEFINED) error("Course found not a root node.");
    state = C_COURSE;

    assert(c); // A Course has not yet been allocated, cannot create course!

    if(attr.find("name") != attr.end()) c->name = attr["name"];
    if(attr.find("title") != attr.end()) c->title = attr["title"];
    if(attr.find("version") != attr.end()) c->version = attr["version"];

    return;
  }

  if(name == "template") {
    if(state != C_COURSE) error("template found outside course.");
    state = C_TEMPLATE;

    assert(c); // A Course has not yet been allocated, cannot create template!
    
    Template t;

    if(attr.find("name") != attr.end()) t.name = attr["name"];
    if(attr.find("title") != attr.end()) t.title = attr["title"];

    c->templates.push_back(t);

    return;
  }

  error("Unknown/illegal tag: %s", name.c_str());
}

void CourseParser::endTag(std::string name)
{
  if(name == "course") state = C_UNDEFINED;
  if(name == "template") state = C_COURSE;
}

int CourseParser::readData(char *data, size_t size)
{
  if(fd == -1) {
    ERR(course, "Invalid file descriptor.\n");
    return 0;
  }
  ssize_t r = read(fd, data, size);
  if(r == -1) {
    ERR(course, "Could not read...%s\n", strerror(errno));
    return 0;
  }
  return r;
}

void CourseParser::parseError(const char *buf, size_t len, std::string error,
                                int lineno)
{
  fprintf(stderr, "CourseParser[%s] error at line %d: %s\n",
          file.c_str(), lineno, error.c_str());
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

Course *CourseParser::getCourse()
{
  return c;
}

#ifdef TEST_COURSEPARSER
//deps: saxparser.cc debug.cc log.cc exception.cc mutex.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include "test.h"

TEST_BEGIN;

CourseParser p("../xml/courses/test.xml");
p.parse();

Course *c = p.getCourse();
TEST_NOTEQUAL(c, nullptr, "Got one?");

TEST_END;

#endif/*TEST_COURSEPARSER*/

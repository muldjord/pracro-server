/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sessionheaderparser.cc
 *
 *  Thu Aug  9 09:06:32 CEST 2012
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
#include "sessionheaderparser.h"

/*
<session timestamp="1234567890"
         status="readonly""
         id="12345"
         template="amd_forunders"
         patientid="0000000000">
   ...
</session>
*/

#include <stdio.h>

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

#include <hugin.hpp>
#include "configuration.h"
#include "exception.h"

void SessionHeaderParser::error(const char* fmt, ...)
{
  ERR(session, "Error in SessionHeaderParser: ");

  {
    char *p;
    va_list argp;
    va_start(argp, fmt);
    if(vasprintf(&p, fmt, argp) != -1) {
      ERR(session, "%s", p);
      throw Exception("Error in SessionHeaderParser: " + std::string(p));
      free(p);
    }
    va_end(argp);
  }

}

SessionHeaderParser::SessionHeaderParser(std::string sessionfile)
{
  done = false;

  file = sessionfile;

  DEBUG(session, "Using session file: %s\n", sessionfile.c_str());

  fd = open(sessionfile.c_str(), O_RDONLY);
  if(fd == -1) error("Could not open file %s. Errno was %i, %s", sessionfile.c_str(), errno, strerror(errno));
}

SessionHeaderParser::~SessionHeaderParser()
{
  if(fd != -1) close(fd);
}

void SessionHeaderParser::startTag(std::string name, attributes_t &attr)
{
  if(done) return;

  if(name == "session") {
    done = true;
    if(attr.find("patientid") != attr.end()) {
      header.patientid = attr["patientid"];
    }

    if(attr.find("template") != attr.end()) {
      header.templ = attr["template"];
    }

    if(attr.find("id") != attr.end()) {
      header.id = attr["id"];
    }
  } else {
    throw Exception("Missing root tag 'session' - found '" + name + "'");
  }
}

int SessionHeaderParser::readData(char *data, size_t size)
{
  if(done) return 0; // If done is true we already found what we were looking
                     //  for and can dismiss the rest of the document.

  if(fd == -1) {
    ERR(session, "Invalid file descriptor.\n");
    return 0;
  }
  ssize_t r = read(fd, data, size);
  if(r == -1) {
    ERR(session, "Could not read...%s\n", strerror(errno));
    return 0;
  }
  return r;
}

void SessionHeaderParser::parseError(const char *buf, size_t len, std::string error, int lineno)
{
  if(done) return; // If done is true we already found what we were looking
                   //  for and can dismiss the rest of the document.

  ERR(session, "SessionHeaderParser[%s] error at line %d: %s\n",
          file.c_str(), lineno, error.c_str());
  ERR(session, "\tBuffer %u bytes: [", (int)len);
  if(fwrite(buf, len, 1, stderr) != len) {}
  ERR(session, "]\n");

  char *slineno;
  if(asprintf(&slineno, " at line %d\n", lineno) != -1) {
    throw Exception(error + slineno);
    free(slineno);
  }
}

SessionHeaderParser::Header SessionHeaderParser::getHeader()
{
  return header;
}

#ifdef TEST_SESSIONHEADERPARSER
//deps: saxparser.cc debug.cc log.cc exception.cc mutex.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(EXPAT_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_SESSIONHEADERPARSER*/

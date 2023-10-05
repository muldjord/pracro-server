/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            templateheaderparser.cc
 *
 *  Thu Jul 30 08:53:26 CEST 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#include "templateheaderparser.h"

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

void TemplateHeaderParser::error(const char* fmt, ...)
{
  ERR(templateparser, "Error in TemplateHeaderParser: ");

  {
    char *p;
    va_list argp;
    va_start(argp, fmt);
    if(vasprintf(&p, fmt, argp) != -1) {
      ERR(templateparser, "%s", p);
      throw Exception("Error in TemplateHeaderParser: " + std::string(p));
      free(p);
    }
    va_end(argp);
  }

}

TemplateHeaderParser::TemplateHeaderParser(std::string templatefile)
{
  t = nullptr;

  file = templatefile;

  DEBUG(templateparser, "Using template file: %s\n", templatefile.c_str());

  fd = open(templatefile.c_str(), O_RDONLY);
  if(fd == -1) error("Could not open file %s. Errno was %i, %s", templatefile.c_str(), errno, strerror(errno));
}

TemplateHeaderParser::~TemplateHeaderParser()
{
  if(fd != -1) close(fd);
  if(t) delete t;
}

void TemplateHeaderParser::startTag(std::string name, attributes_t &attr)
{
  if(t) return;

  if(name == "template") {
    t = new Template();

    if(attr.find("name") != attr.end()) t->name = attr["name"];
    if(attr.find("title") != attr.end()) t->title = attr["title"];
    if(attr.find("version") != attr.end()) t->version = attr["version"];

  } else {
    throw Exception("Missing root tag 'template' - found '" + name + "'");
  }
}

int TemplateHeaderParser::readData(char *data, size_t size)
{
  // If t is allocated, it means that we have parsed the template
  // tag, and can dismiss the rest of the document.
  if(t) return 0; 

  if(fd == -1) {
    ERR(templateparser, "Invalid file descriptor.\n");
    return 0;
  }
  ssize_t r = read(fd, data, size);
  if(r == -1) {
    ERR(templateparser, "Could not read...%s\n", strerror(errno));
    return 0;
  }
  return r;
}

void TemplateHeaderParser::parseError(const char *buf, size_t len,
                                      std::string error, int lineno)
{
  // Ignore "unclosed token" errors when the template tag has been parsed.
  if(t) return; 

  ERR(templateparser, "TemplateHeaderParser[%s] error at line %d: %s\n",
          file.c_str(), lineno, error.c_str());
  ERR(templateparser, "\tBuffer %u bytes: [", (int)len);
  if(fwrite(buf, len, 1, stderr) != len) {}
  ERR(templateparser, "]\n");

  char *slineno;
  if(asprintf(&slineno, " at line %d\n", lineno) != -1) {
    throw Exception(error + slineno);
    free(slineno);
  }
}

Template *TemplateHeaderParser::getTemplate()
{
  return t;
}

#ifdef TEST_TEMPLATEHEADERPARSER
//deps: saxparser.cc exception.cc debug.cc log.cc mutex.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#define XMLFILE "/tmp/test_templateheaderparser.xml"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
/*
static char xml[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<template name=\"testtemplate\" version=\"1.0\">\n"
" <sometag/>\n"
" <someothertag/>\n"
"</template>"
;

static char xml_nontemplate[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<dims name=\"testtemplate\" version=\"1.0\">\n"
" <sometag/>\n"
" <someothertag/>\n"
"</dims>"
;

static char xml_fail[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<template name\"testtemplate\" version=\"1.0\">\n"
" <sometag/>\n"
" <someothertag/>\n"
"</template>"
;
*/
TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  FILE *fp = fopen(XMLFILE, "w");
  if(!fp) {
    printf("Could not write to %s\n", XMLFILE);
    return 1;
  }
fprintf(fp, "%s", xml);
  fclose(fp);

  {
    // Test parsing of correct template xml data.
    TemplateHeaderParser parser(XMLFILE);
    try {
      parser.parse();
    } catch(Exception &e) {
      printf("Failed to parse: %s\n", e.what());
      return 1;
    }
  }  

  fp = fopen(XMLFILE, "w");
  if(!fp) {
    printf("Could not write to %s\n", XMLFILE);
    return 1;
  }
fprintf(fp, "%s", xml_nontemplate);
  fclose(fp);

  // Test parsing of correct xml data, but not template (should throw an exception).
  {
    TemplateHeaderParser parser(XMLFILE);
    try {
      parser.parse();
    } catch(Exception &e) {
      printf("Failed to parse: %s\n", e.what());
      goto onandon;
    }
    return 1;
  }
  onandon:

  fp = fopen(XMLFILE, "w");
  if(!fp) {
    printf("Could not write to %s\n", XMLFILE);
    return 1;
  }
fprintf(fp, "%s", xml_fail);
  fclose(fp);

  // Test parsing of invalid xml data (should throw an exception).
  {
    TemplateHeaderParser parser(XMLFILE);
    try {
      parser.parse();
    } catch(Exception &e) {
      printf("Failed to parse: %s\n", e.what());
      goto yetonandon;
    }
    return 1;
  }
  yetonandon:

  unlink(XMLFILE);
*/
TEST_END;

#endif/*TEST_TEMPLATEHEADERPARSER*/

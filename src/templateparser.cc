/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            templateparser.cc
 *
 *  Mon May 12 08:36:24 CEST 2008
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
#include "templateparser.h"

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

void TemplateParser::error(const char* fmt, ...)
{
  va_list argp;
  char *p;
  va_start(argp, fmt);
  vasprintf(&p, fmt, argp);
  va_end(argp);
  ERR(template, "Error in TemplateParser: %s\n", p);
  throw Exception(std::string("Error in TemplateParser: ") + p);
  free(p);
}

TemplateParser::TemplateParser(std::string templatefile)
{
  state = UNDEFINED;
  t = new Template();
  current_macro = nullptr;

  file = templatefile;

  DEBUG(template, "Using template file: %s\n", file.c_str());

  fd = open(file.c_str(), O_RDONLY);
  if(fd == -1) error("Could not open file %s. Errno was %i, %s", file.c_str(), errno, strerror(errno));
}

TemplateParser::~TemplateParser()
{
  if(fd != -1) close(fd);
  delete t;
}

void TemplateParser::characterData(std::string &data)
{
  if(state == SCRIPT) {
    assert(current_script); // No script present!
    current_script->code.append(data);
  }
}

void TemplateParser::startTag(std::string name, attributes_t &attr)
{
  // Enable macro parsing
  if(name == "template") {
    if(state != UNDEFINED) error("Template found not a root node.");
    state = TEMPLATE;

    assert(t); // A Template has not yet been allocated, cannot create template!

    if(attr.find("name") != attr.end()) t->name = attr["name"];
    if(attr.find("title") != attr.end()) t->title = attr["title"];
    if(attr.find("version") != attr.end()) t->version = attr["version"];

    return;
  }

  // Create macro and enable parsing of queries, maps and widgets
  if(name == "macro" || name == "header") {
    if(state != TEMPLATE) error("macro found outside template.");
    state = MACRO;

    assert(t); // A Template has not yet been allocated, cannot create macro!
    
    Macro m;

    if(attr.find("name") != attr.end()) m.name = attr["name"];
    if(attr.find("version") != attr.end()) m.version = attr["version"];
    if(attr.find("caption") != attr.end()) m.caption = attr["caption"];
    if(attr.find("requires") != attr.end()) m.requires = attr["requires"];
    if(attr.find("ttl") != attr.end()) m.ttl = attr["ttl"];

    m.isImportant = attr.find("important") != attr.end() &&
      attr["important"] == "true";
    m.isStatic = attr.find("static") != attr.end() && attr["static"] == "true";
    m.isCompact = attr.find("compact") != attr.end() &&
      attr["compact"] == "true";

    m.isHeader = name == "header";
    t->macros.push_back(m);
    current_macro = &(t->macros.back());

    return;
  }

  // Enable script parsing
  if(name == "scripts") {
    if(state != TEMPLATE) error("scripts found outside template.");
    state = SCRIPTS;

    assert(t); // No template is currently available, cannot create maps!

    return;
  }

  // Create script
  if(name == "script") {

    assert(t); // No template is currently available, cannot create script!

    switch(state) {
    case SCRIPTS:
      {
        state = SCRIPT;

        Script s;
        s.attributes = attr;
        t->scripts.push_back(s);
        current_script = &(t->scripts.back());
      }
      break;
    default:
       error("<script> tag found outside <scripts> tag.");
       break;
    }
    return;
  }

  error("Unknown/illegal tag: %s", name.c_str());
}

void TemplateParser::endTag(std::string name)
{
  if(name == "template") state = UNDEFINED;
  if(name == "macro" || name == "header") {
    current_macro = nullptr;
    state = TEMPLATE;
  }
  if(name == "scripts") state = TEMPLATE;
  if(name == "script") {
    switch(state) {
    case SCRIPT:
      current_script = nullptr;
      state = SCRIPTS;
      break;
    default:
      // tag mismatch?
      break;
    }
  }
}

int TemplateParser::readData(char *data, size_t size)
{
  if(fd == -1) {
    ERR(template, "Invalid file descriptor.\n");
    return 0;
  }
  ssize_t r = read(fd, data, size);
  if(r == -1) {
    ERR(template, "Could not read...%s\n", strerror(errno));
    return 0;
  }
  return r;
}

void TemplateParser::parseError(const char *buf, size_t len, std::string error,
                                int lineno)
{
  fprintf(stderr, "TemplateParser[%s] error at line %d: %s\n",
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

Template *TemplateParser::getTemplate()
{
  return t;
}

#ifdef TEST_TEMPLATEPARSER
//deps: saxparser.cc debug.cc log.cc exception.cc mutex.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#define XMLFILE "/tmp/test_templateparser.xml"

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
" <macro name=\"mymacro\"/>\n"
" <header caption=\"mycaption\"/>\n"
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
    TemplateParser parser(XMLFILE);
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
    TemplateParser parser(XMLFILE);
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
    TemplateParser parser(XMLFILE);
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

#endif/*TEST_TEMPLATEPARSER*/

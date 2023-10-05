/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            macroparser.cc
 *
 *  Wed Jun  4 11:57:38 CEST 2008
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
#include <hugin.hpp>
#include "macroparser.h"
#include "configuration.h"

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

#ifndef XML
// For XML
#include <config.h>
#endif/*XML*/

#include <errno.h>
#include <string.h>

#include "exception.h"

void MacroParser::error(const char* fmt, ...)
{
  ERR(macro, "Error in MacroParser: ");

  {
    char *p;
    va_list argp;
    va_start(argp, fmt);
    if(vasprintf(&p, fmt, argp) != -1) {
      ERR(macro, "%p", p);
      throw Exception("Error in MacroParser: " + std::string(p));
      free(p);
    }
    va_end(argp);
  }

}

MacroParser::MacroParser(std::string macrofile)
{
  state = UNDEFINED;
  m = nullptr;
  current_map = nullptr;
  current_script = nullptr;
  current_resume_script = nullptr;

  file = macrofile;

  DEBUG(macro, "Using macro file: %s\n", file.c_str());

  fd = open(file.c_str(), O_RDONLY);
  if(fd == -1) error("Could not open file %s. Errno was %i, %s", file.c_str(), errno, strerror(errno));
}

MacroParser::~MacroParser()
{
  if(fd != -1) close(fd);
  if(m) delete m;
}

void MacroParser::characterData(std::string &data)
{
  if(state == RESUME_SCRIPT) {
    assert(current_resume_script); // No macro present!
    current_resume_script->code.append(data);
  }

  if(state == MAP) {
    assert(current_map); // No map present!
    current_map->attributes["lua"].append(data);
  }

  if(state == SCRIPT) {
    assert(current_script); // No script present!
    current_script->code.append(data);
  }

  if(state == COMMIT_SCRIPT) {
    assert(current_commit_script); // No script present!
    current_commit_script->code.append(data);
  }
}

void MacroParser::startTag(std::string name, attributes_t &attr)
{
  // Create macro and enable parsing of queries, maps and widgets
  if(name == "macro") {
    if(state != UNDEFINED) error("macro found not root tag.");
    state = MACRO;

    assert(!m); // A Macro has already been allocated, cannot create macro!
    
    m = new Macro();

    if(attr.find("name") != attr.end()) m->name = attr["name"];
    if(attr.find("version") != attr.end()) m->version = attr["version"];

    return;
  }

  // Enable resume parsing
  if(name == "resume") {
    if(state != MACRO) error("resume found outside macro.");
    state = RESUME;

    m->resume.attributes = attr;

    assert(m); // No macro is currently available, cannot create resume!

    return;
  }

  // Enable oncommit parsing
  if(name == "oncommit") {
    if(state != MACRO) error("oncommit found outside macro.");
    state = COMMIT_SCRIPTS;

    m->resume.attributes = attr;

    assert(m); // No macro is currently available, cannot create resume!

    return;
  }

  // Enable Query parsing
  if(name == "queries") {
    if(state != MACRO) error("queries found outside macro.");
    state = QUERIES;
    
    assert(m); // No macro is currently available, cannot create queries!

    return;
  }

  // Create Query
  if(name == "query") {
    if(state != QUERIES) error("query found outside queries.");
    state = QUERY;

    assert(m); // No macro is currently available, cannot create query!

    Query q;
    q.attributes = attr;
    m->queries.push_back(q);

    return;
  }

  // Enable Map parsing
  if(name == "maps") {
    if(state != MACRO) error("maps found outside macro.");
    state = MAPS;

    assert(m); // No macro is currently available, cannot create maps!

    return;
  }

  // Create Query
  if(name == "map") {
    if(state != MAPS) error("map found outside maps.");
    state = MAP;

    assert(m); // No macro is currently available, cannot create map!

    Map map;
    map.attributes = attr;
    m->maps.push_back(map);
    current_map = &(m->maps.back());

    return;
  }

  // Enable script parsing
  if(name == "scripts") {
    if(state != MACRO) error("scripts found outside macro.");
    state = SCRIPTS;

    assert(m); // No macro is currently available, cannot create maps!

    return;
  }

  // Create script
  if(name == "script") {

    assert(m); // No macro is currently available, cannot create script!

    switch(state) {
    case SCRIPTS:
      {
        state = SCRIPT;

        Script s;
        s.attributes = attr;
        m->scripts.push_back(s);
        current_script = &(m->scripts.back());
      }
      break;
    case RESUME:
      {
        state = RESUME_SCRIPT;

        Script s;
        s.attributes = attr;
        m->resume_scripts.push_back(s);
        current_resume_script = &(m->resume_scripts.back());
      }
      break;
    case COMMIT_SCRIPTS:
      {
        state = COMMIT_SCRIPT;

        Script s;
        s.attributes = attr;
        m->commit_scripts.push_back(s);
        current_commit_script = &(m->commit_scripts.back());
      }
      break;
    default:
       error("<script> tag found outside <scripts>, <commitscripts> or <resume> tags.");
       break;
    }
    return;
  }

  // Enable widget parsing
  if(name == "widgets") {

    if(state != MACRO) error("widgets found outside macro.");
    state = WIDGETS;

    assert(m); // No macro is currently available, cannot create widgets!
  
    m->widgets.attributes = attr;
    m->widgets.attributes["tagname"] = name;

    Widget *current = &(m->widgets);
    widgetstack.push_back(current);

    return;
  }

  // TODO: We need to parse some (maybe even all) widgets in order to
  //       make db lookup of the previous values.
  if(state == WIDGETS) {

    assert(widgetstack.size()); // Widget stack is empty, cannot create!

    Widget w;
    w.attributes = attr;
    w.attributes["tagname"] = name;

    Widget *parent = widgetstack.back();
    parent->widgets.push_back(w);
    
    Widget *current = &(parent->widgets.back());
    widgetstack.push_back(current);

    return;
  }


  // Handle include
  if(name == "include") {
    return;
  }

  error("Unknown/illegal tag: %s", name.c_str());
}

void MacroParser::endTag(std::string name)
{
  if(name == "macro") {
    state = UNDEFINED;
  }
  if(name == "resume") state = MACRO;
  if(name == "oncommit") state = MACRO;
  if(name == "queries") state = MACRO;
  if(name == "query") state = QUERIES;
  if(name == "maps") state = MACRO;
  if(name == "map") {
    current_map = nullptr;
    state = MAPS;
  }
  if(name == "scripts") state = MACRO;
  if(name == "script") {
    switch(state) {
    case SCRIPT:
      current_script = nullptr;
      state = SCRIPTS;
      break;

    case RESUME_SCRIPT:
      current_resume_script = nullptr;
      state = RESUME;
      break;

    case COMMIT_SCRIPT:
      current_commit_script = nullptr;
      state = COMMIT_SCRIPTS;
      break;

    default:
      // tag mismatch?
      break;
    }
  }
  if(name == "widgets") state = MACRO;
  
  if(state == WIDGETS) {
    assert(widgetstack.size()); // Widget stack is empty, cannot pop!
    widgetstack.pop_back();
    if(widgetstack.size() == 0) state = MACRO;
  }
}

int MacroParser::readData(char *data, size_t size)
{
  if(fd == -1) {
    ERR(macro, "Invalid file descriptor.\n");
    return 0;
  }
  ssize_t r = read(fd, data, size);
  if(r == -1) {
    ERR(macro, "Could not read...%s\n", strerror(errno));
    return 0;
  }
  return r;
}

void MacroParser::parseError(const char *buf, size_t len, std::string error, int lineno)
{
  ERR(macro, "MacroParser[%s] error at line %d: %s\n",
          file.c_str(), lineno, error.c_str());
  ERR(macro, "\tBuffer %u bytes: [", (int)len);
  if(fwrite(buf, len, 1, stderr) != len) {}
  ERR(macro, "]\n");

  char *slineno;
  if(asprintf(&slineno, " at line %d\n", lineno) != -1) {
    throw Exception(error + slineno);
    free(slineno);
  }
}

Macro *MacroParser::getMacro()
{
  return m;
}

#ifdef TEST_MACROPARSER
//deps: debug.cc log.cc saxparser.cc exception.cc mutex.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#define XMLFILE "/tmp/test_macroheaderparser.xml"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

/*
static char xml[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<macro name=\"testmacro\" version=\"1.0\">\n"
" <resume/>\n"
" <queries/>\n"
" <maps/>\n"
" <scripts/>\n"
" <widgets/>\n"
"</macro>"
;

static char xml_nonmacro[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<dims name=\"testmacro\" version=\"1.0\">\n"
" <sometag/>\n"
" <someothertag/>\n"
"</dims>"
;

static char xml_fail[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<macro name\"testmacro\" version=\"1.0\">\n"
" <sometag/>\n"
" <someothertag/>\n"
"</macro>"
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
    // Test parsing of correct macro xml data.
    MacroParser parser(XMLFILE);
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
fprintf(fp, "%s", xml_nonmacro);
  fclose(fp);

  // Test parsing of correct xml data, but not macro (should throw an exception).
  {
    MacroParser parser(XMLFILE);
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
    MacroParser parser(XMLFILE);
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

#endif/*TEST_MACROPARSER*/

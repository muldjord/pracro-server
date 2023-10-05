/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            fieldnamescanner.cc
 *
 *  Wed Jan 26 09:21:58 CET 2011
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
#include "fieldnamescanner.h"

#include <stdio.h>

#include "util.h"

#include "macroparser.h"
#include "templateparser.h"
#include "configuration.h"
#include "exception.h"
#include "macrolist.h"

#include <hugin.hpp>

fieldnames_t getFields(Widget &widget)
{
  fieldnames_t fieldnames;
  
  if(widget.attributes.find("name") != widget.attributes.end()) {
    fieldnames.push_back(widget.attributes["name"]);
    //printf("\t\t%s\n", widget.attributes["name"].c_str());
  }

  std::vector< Widget >::iterator i = widget.widgets.begin();
  while(i != widget.widgets.end()) {
    fieldnames_t f = getFields(*i);
    fieldnames.insert(fieldnames.end(), f.begin(), f.end());
    i++;
  }

  return fieldnames;
}

templates_t scanfieldnames(Environment &env, std::set<std::string> &filter)
{
  templates_t templates;

  //  TemplateList templatelist(Conf::xml_basedir + "/templates");
  //  MacroList macrolist(Conf::xml_basedir + "/macros");

  // Iterate templates:
  std::vector<std::string> templatefiles = getTemplates();
  std::vector<std::string>::iterator tfs = templatefiles.begin();
  while(tfs != templatefiles.end()) {
    std::string templ = tfs->substr(0, tfs->length() - 4);
    fieldnames_t fieldnames;
    DEBUG(scanner, "%s:\n", tfs->c_str());
    TemplateParser parser(Conf::xml_basedir + "/templates/" + *tfs);
    try {
      parser.parse();
      Template *t = parser.getTemplate();
      if(!t) ERR(scanner, "Missing template!\n");
      // Iterate macros:
      std::vector<Macro>::iterator ms = t->macros.begin();
      while(ms != t->macros.end()) {
        if(ms->isHeader == false) {
          std::string macro = ms->name;
          DEBUG(scanner, "Name '%s'\n", macro.c_str());
          std::string macrofile = env.macrolist.getLatestVersion(macro);
          DEBUG(scanner, "File '%s'\n", macrofile.c_str());

          // Iterate fields:
          MacroParser parser(macrofile);
          try {
            parser.parse();
            Macro *m = parser.getMacro();
            fieldnames_t f = getFields(m->widgets);
            fieldnames.insert(fieldnames.end(), f.begin(), f.end());
          } catch( Exception &e ) {
            ERR(scanner, "Error reading macro: %s: %s\n",
                macrofile.c_str(), e.what());
          }
        }
          
        ms++;
      }
    } catch( Exception &e ) {
      ERR(scanner, "Error reading template: %s: %s\n", tfs->c_str(), e.what());
    }

    fieldnames_t nodubs;
    fieldnames_t::iterator fi = fieldnames.begin();
    while(fi != fieldnames.end()) {
      bool hasdub = false;
      fieldnames_t::iterator di = nodubs.begin();
      while(di != nodubs.end()) {
        if(*di == *fi) {
          hasdub = true;
          break;
        }
        di++;
      }

      if(!hasdub && filter.find(*fi) != filter.end()) nodubs.push_back(*fi);
      fi++;
    }
    
    templates[templ] = nodubs;
    tfs++;
  }

  return templates;
}

#ifdef TEST_FIELDNAMESCANNER
//deps: util.cc configuration.cc debug.cc log.cc templateparser.cc exception.cc saxparser.cc macrolist.cc entitylist.cc inotify.cc mutex.cc macroheaderparser.cc versionstr.cc macroparser.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS) $(ATF_CFLAGS) $(OPENSSL_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS) $(ATF_LIBS) $(OPENSSL_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
Conf::xml_basedir = "../../xml";
std::set<std::string> filter;
filter.insert("dims");
filter.insert("test1");
templates_t t = scanfieldnames(filter);

templates_t::iterator ti = t.begin();
while(ti != t.end()) {
  printf("%s\n", ti->first.c_str());
  fieldnames_t::iterator fi = ti->second.begin();
  while(fi != ti->second.end()) {
    printf("\t%s\n", (*fi).c_str());
    fi++;
  }
  ti++;
}
*/

TEST_END;

#endif/*TEST_FIELDNAMESCANNER*/

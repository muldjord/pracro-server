/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macrotool_dump.cc
 *
 *  Mon Jul  6 08:37:22 CEST 2009
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
#include "dump.h"

#include <vector>
#include <map>
#include <set>

#include <config.h>

#include <stdio.h>

#include "util.h"

#include <hugin.h>

#include "macroparser.h"
#include "templateparser.h"

#include "configuration.h"

#include "exception.h"

#include "database.h"

struct _macro {
  std::string name;
  std::string title;
  std::set<std::string>  templates;
  std::map<std::string, bool> fields;
  std::string file;
  std::string version;
};

struct _template {
  std::string name;
  std::string title;
  std::vector<std::string> macros;
  std::string file;
  std::string version;
};

static const char usage_str[] =
"  help        Prints this helptext.\n"
"  macros      Writes macro names, versions, filenames, titles and template references to stdout.\n"
"  templates   Writes template names, versions, filenames, titles and macro references to stdout.\n"
"  fields      Outputs all fields sorted by macro, with indication of those in the fieldnames table.\n"
;

static std::map<std::string, bool> getFields(Widget &widget)
{
  std::map<std::string, bool> fields;

  std::vector< Widget >::iterator w = widget.widgets.begin();
  while(w != widget.widgets.end()) {
    std::map<std::string, bool> fs = getFields(*w);
    fields.insert(fs.begin(), fs.end());
    w++;
  }

  if(widget.attributes.find("name") != widget.attributes.end())
    fields[widget.attributes["name"]] =
      widget.attributes.find("value") != widget.attributes.end();

  return fields;
}

static std::map<std::string, struct _macro> macroList()
{
  std::map<std::string, struct _macro> macros;

  std::vector<std::string> macrofiles = getMacros();
  std::vector<std::string>::iterator mfs = macrofiles.begin();
  while(mfs != macrofiles.end()) {
    //std::string name = mfs->substr(0, mfs->length() - 4);

    MacroParser parser(Conf::xml_basedir + "/macros/" + *mfs);
    try {
      parser.parse();
      Macro *macro = parser.getMacro();
      
      std::string key = macro->name;// + "-" + macro->version;
      macros[key].name = macro->name;
      macros[key].file = *mfs;
      macros[key].title = macro->widgets.attributes["caption"];
      macros[key].fields = getFields(macro->widgets);
      macros[key].version = macro->version;
    } catch( Exception &e ) {
      printf("Skipping: %s: %s\n", mfs->c_str(), e.what());
    }

    mfs++;
  }
  
  std::vector<std::string> templatefiles = getTemplates();
  std::vector<std::string>::iterator tfs = templatefiles.begin();
  while(tfs != templatefiles.end()) {
    std::string templ = tfs->substr(0, tfs->length() - 4);
    TemplateParser parser(Conf::xml_basedir + "/templates/" + *tfs);
    try {
      parser.parse();
      Template *t = parser.getTemplate();
      std::vector<Macro>::iterator ms = t->macros.begin();
      while(ms != t->macros.end()) {
        if(ms->isHeader) macros[ms->name].templates.insert(templ);
        ms++;
      }
    } catch( Exception &e ) {
      printf("Skipping: %s: %s\n", tfs->c_str(), e.what());
    }

    tfs++;
  }

  return macros;
}

static void dump_fields()
{
  Database db("pgsql", Conf::database_addr, "", Conf::database_user,
              Conf::database_passwd, "");
  std::vector<Fieldname> fieldnames = db.getFieldnames();

  std::map<std::string, struct _macro> macros = macroList();
  std::map<std::string, struct _macro>::iterator ms = macros.begin();
  while(ms != macros.end()) {
    printf("Macro: %s\n", ms->second.name.c_str());

    std::map<std::string, bool>::iterator ts = ms->second.fields.begin();
    while(ts != ms->second.fields.end()) {
      bool reg = false;
      bool value = ts->second;

      std::vector<Fieldname>::iterator fs = fieldnames.begin();
      while(fs != fieldnames.end()) {
        if(ts->first == fs->name) {
          reg = true;
          break;
        }
        fs++;
      }
      printf("\t%s %s [%s]\n",
             reg?"(*)":"   ", ts->first.c_str(), value?"+":"-");
      ts++;
    }

    printf("\n");
    ms++;
  }

  printf("----\n");
  printf("(*) Indicates that the field exists in the fieldnames table.\n");
  printf("[-] Indicates that the field does not have a value field.\n");
  printf("[+] Indicates that the field does have a value field.\n");
}

static void dump_macros()
{
  std::map<std::string, struct _macro> macros = macroList();
  macros[""].title = "Title:";
  macros[""].file = "File:";
  macros[""].version = "Version:";
  macros[""].name = "Macro:";
  macros[""].templates.insert("Templates:");

  size_t name_sz = 0;
  size_t version_sz = 0;
  size_t file_sz = 0;
  size_t title_sz = 0;

  std::map<std::string, struct _macro>::iterator ms = macros.begin();
  while(ms != macros.end()) {
    if(ms->second.name.length() > name_sz) name_sz = ms->second.name.length();
    if(ms->second.version.length() > version_sz) version_sz = ms->second.version.length();
    if(ms->second.file.length() > file_sz) file_sz = ms->second.file.length();
    if(ms->second.title.length() > title_sz) title_sz = ms->second.title.length();
    ms++;
  }

  ms = macros.begin();
  while(ms != macros.end()) {
    printcolumn(ms->second.name, name_sz);
    printcolumn(ms->second.version, version_sz);
    printcolumn(ms->second.file, file_sz);
    printcolumn(ms->second.title, title_sz);

    std::set<std::string>::iterator ts = ms->second.templates.begin();
    while(ts != ms->second.templates.end()) {
      if(ts != ms->second.templates.begin()) printf(", ");
      printf("%s", ts->c_str() );
      ts++;
    }

    printf("\n");
    ms++;
  }
}

static std::map<std::string, struct _template> templateList()
{
  std::map<std::string, struct _template> templates;
  
  std::vector<std::string> templatefiles = getTemplates();
  std::vector<std::string>::iterator tfs = templatefiles.begin();
  while(tfs != templatefiles.end()) {
    TemplateParser parser(Conf::xml_basedir + "/templates/" + *tfs);
    try {
      parser.parse();
      Template *t = parser.getTemplate();

      std::string key = t->name;

      templates[key].file = *tfs;
      templates[key].name = t->name;
      templates[key].title = t->title;
      templates[key].version = t->version;
    
      std::vector<Macro>::iterator ms = t->macros.begin();
      while(ms != t->macros.end()) {
        if(ms->isHeader) templates[key].macros.push_back(ms->name);
        ms++;
      }
    } catch( Exception &e ) {
      printf("Skipping: %s: %s\n", tfs->c_str(), e.what());
    }
    tfs++;
  }

  return templates;
}

static void dump_templates()
{
  std::map<std::string, struct _template> templates = templateList();
  templates[""].title = "Title:";
  templates[""].file = "File:";
  templates[""].version = "Version:";
  templates[""].name = "Template:";

  size_t name_sz = 0;
  size_t version_sz = 0;
  size_t file_sz = 0;
  size_t title_sz = 0;

  std::map<std::string, struct _template>::iterator ts = templates.begin();
  while(ts != templates.end()) {
    if(ts->second.name.length() > name_sz) name_sz = ts->second.name.length();
    if(ts->second.version.length() > version_sz) version_sz = ts->second.version.length();
    if(ts->second.file.length() > file_sz) file_sz = ts->second.file.length();
    if(ts->second.title.length() > title_sz) title_sz = ts->second.title.length();
    ts++;
  }

  ts = templates.begin();
  while(ts != templates.end()) {
    printcolumn(ts->second.name, name_sz);
    printcolumn(ts->second.version, version_sz);
    printcolumn(ts->second.file, file_sz);
    printcolumn(ts->second.title, title_sz);

    printf("\n");
    std::vector<std::string>::iterator ms = ts->second.macros.begin();
    while(ms != ts->second.macros.end()) {
      printf("\t%s\n", ms->c_str() );
      ms++;
    }

    printf("\n");
    ts++;
  }
}

void macrotool_dump(std::vector<std::string> params)
{
  if(params.size() < 1) {
    printf("%s", usage_str);
    return;
  }

  DEBUG(fieldnames, "dump: %s\n", params[0].c_str());

  if(params[0] == "fields") {
    if(params.size() != 1) {
      printf("The command 'fields' doen't take any parameters.\n");
      printf("%s", usage_str);
      return;
    }
    dump_fields();
    return;
  }

  if(params[0] == "macros") {
    if(params.size() != 1) {
      printf("The command 'macro' doen't take any parameters.\n");
      printf("%s", usage_str);
      return;
    }
    dump_macros();
    return;
  }

  if(params[0] == "templates") {
    if(params.size() != 1) {
      printf("The command 'templates' doen't take any parameters.\n");
      printf("%s", usage_str);
      return;
    }
    dump_templates();
    return;
  }

  if(params[0] == "help") {
    printf("%s", usage_str);
    return;
  }

  printf("Unknown command '%s'\n", params[0].c_str());
  printf("%s", usage_str);
  return;
}

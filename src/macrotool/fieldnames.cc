/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macrotool_fieldnames.cc
 *
 *  Mon Jul  6 14:15:05 CEST 2009
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
#include "fieldnames.h"

#include <string.h>
#include <stdio.h>

#include "util.h"
#include "macroparser.h"

#include <hugin.h>

#include "database.h"
#include "configuration.h"

static const char usage_str[] =
"  help            Prints this helptext.\n"
"  add name desc   Add a field called 'name', described as 'desc'\n"
"  del name        Delete a field called 'name'\n"
"  list            List all fieldnames as well as their macro references.\n"
;

static void add(std::string name, std::string desc)
{
  Database db("pgsql", Conf::database_addr, "", Conf::database_user, Conf::database_passwd, "");
  db.addFieldname(name, desc);
}

static void del(std::string name)
{
  Database db("pgsql", Conf::database_addr, "", Conf::database_user, Conf::database_passwd, "");
  db.delFieldname(name);
}

static std::vector<std::string> getWidgetNames(Widget &w)
{
  std::vector<std::string> v;
  if(w.attributes.find("name") != w.attributes.end()) v.push_back(w.attributes["name"]);

  std::vector< Widget >::iterator i = w.widgets.begin();
  while(i != w.widgets.end()) {
    std::vector<std::string> _v = getWidgetNames(*i);
    v.insert(v.end(), _v.begin(), _v.end());
    i++;
  }

  return v;
}

static std::map<std::string, std::vector<std::string> > getMacroRefsList()
{
  std::map<std::string, std::vector<std::string> > reflist;

  std::vector<std::string> macrofiles = getMacros();
  std::vector<std::string>::iterator mfs = macrofiles.begin();
  while(mfs != macrofiles.end()) {
    std::string name = mfs->substr(0, mfs->length() - 4);

    MacroParser parser(Conf::xml_basedir + "/macros/" + *mfs);
    parser.parse();
    Macro *macro = parser.getMacro();

    std::string key = name;
    reflist[key] = getWidgetNames(macro->widgets);

    mfs++;
  }

  return reflist;
}

static std::vector<std::string> getMacroRefs(std::map<std::string, std::vector<std::string> > reflist, std::string name)
{
  std::vector<std::string> macros;
  std::map<std::string, std::vector<std::string> >::iterator macro = reflist.begin();
  while(macro != reflist.end()) {
    std::vector<std::string>::iterator field = macro->second.begin();
    while(field != macro->second.end()) {
      if(*field == name) macros.push_back(macro->first);
      field++;
    }
    macro++;
  }
  return macros;
}

static void list()
{
  Database db("pgsql", Conf::database_addr, "", Conf::database_user, Conf::database_passwd, "");

  std::vector<Fieldname> fieldnames = db.getFieldnames();

  std::map<std::string, std::vector<std::string> > reflist = getMacroRefsList();
  
  size_t name_sz = 0;
  size_t desc_sz = 0;
  size_t time_sz = 0;

  std::vector<Fieldname>::iterator i = fieldnames.begin();
  while(i != fieldnames.end()) {
    if(i->name.length() > name_sz) name_sz = i->name.length();
    if(i->description.length() > desc_sz) desc_sz = i->description.length();
    char ts[32];
    sprintf(ts, "%u", (unsigned int)i->timestamp);
    if(strlen(ts) > time_sz) time_sz = strlen(ts);
    i++;
  }

  printcolumn("Name:", name_sz);
  printcolumn("Description:", desc_sz);
  printcolumn("Timestamp:", time_sz);
  printf("Macros:");
  printf("\n");

  i = fieldnames.begin();
  while(i != fieldnames.end()) {
    printcolumn(i->name, name_sz);
    printcolumn(i->description, desc_sz);
    char ts[32];
    sprintf(ts, "%u", (unsigned int)i->timestamp);
    printcolumn(ts, time_sz);

    std::vector<std::string> macros = getMacroRefs(reflist, i->name);
    std::vector<std::string>::iterator j = macros.begin();
    while(j != macros.end()) {
      if(j != macros.begin()) printf(", ");
      printf("%s", j->c_str());
      j++;
    }

    printf("\n");
    i++;
  }
}

void macrotool_fieldnames(std::vector<std::string> params)
{
  if(params.size() < 1) {
    printf("%s", usage_str);
    return;
  }

  DEBUG(fieldnames, "fieldnames: %s\n", params[0].c_str());

  if(params[0] == "list") {
    if(params.size() != 1) {
      printf("The command 'list' does not need any parameters.\n");
      printf("%s", usage_str);
      return;
    }
    list();
    return;
  }

  if(params[0] == "add") {
    if(params.size() != 3) {
      printf("The command 'add' needs 2 parameters.\n");
      printf("%s", usage_str);
      return;
    }
    add(params[1], params[2]);
    return;
  }

  if(params[0] == "del") {
    if(params.size() != 2) {
      printf("The command 'del' needs 1 parameter.\n");
      printf("%s", usage_str);
      return;
    }
    del(params[1]);
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

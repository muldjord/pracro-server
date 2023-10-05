/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            export.cc
 *
 *  Wed Jan 26 11:44:12 CET 2011
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
#include "export.h"

#include <config.h>
#include <stdio.h>
#include <hugin.h>

#ifndef WITHOUT_DB

#include <stdlib.h>
#include <pqxx/pqxx>

#include "fieldnamescanner.h"
#include "configuration.h"

#define SEP "\t"

static std::string escape(std::string &str)
{
  std::string out = "\"";
  std::string::iterator i = str.begin();
  while(i != str.end()) {
    if(*i == '\"') out += "''";
    else out += *i;
    i++;
  }
  out += "\"";
  return out;
}

class File {
public:
  File(std::string n, fieldnames_t &f, pqxx::work &w)
    : work(w), name(n), fieldnames(f)
  {
    name += ".csf";
    fp = fopen(name.c_str(), "w");

    pos["id"] = 0;
    pos["patientid"] = 1;
    pos["time"] = 2;
    pos["template"] = 3;

    //printf("%s\n", n.c_str());

    size_t idx = 4;
    fieldnames_t::iterator i = f.begin();
    while(i != f.end()) {
      //printf("%s ", i->c_str());
      pos[*i] = idx;
      idx++;
      i++;
    }
    //    printf("\n");

    output_header();
  }

  ~File()
  {
    if(fp) fclose(fp);
  }

  void output_header()
  {
    beginrow();
    addcell("id", "ID");
    addcell("patientid", "Patient");
    addcell("time", "Time");
    addcell("template", "Template");

    {
      pqxx::result result =
        work.exec("SELECT DISTINCT ON(name) name, caption FROM fieldnames"
                  " WHERE extract='true';");
      pqxx::result::const_iterator ri = result.begin();
      for(unsigned int r = 0; r < result.size(); r++) {
        pqxx::result::tuple tuple = result.at(r); 
        std::string name = tuple.at(0).c_str();
        std::string caption = tuple.at(1).c_str();
        addcell(name, caption);
      }
    }

    endrow();
  }

  void beginrow()
  {
    cells.clear();
    cells.insert(cells.begin(), pos.size(), "");
  }

  void addcell(std::string key, std::string value)
  {
    if(pos.find(key) == pos.end()) return;
    cells[pos[key]] = value;
  }

  void endrow()
  {
    std::vector<std::string>::iterator i = cells.begin();
    while(i != cells.end()) {
      fprintf(fp, "%s"SEP, escape(*i).c_str());
      i++;
    }
    fprintf(fp, "\n");
  }

private:
  pqxx::work &work;
  std::map<std::string, int> pos;
  std::vector<std::string> cells;
  std::string name;
  fieldnames_t &fieldnames;
  FILE *fp;
};


static void export_prefix(std::string prefix)
{

  std::map<std::string, File *> files;

  if(Conf::database_backend != "pgsql") {
    printf("ERROR: Export only available for the pgsql database backend.\n");
    return;
  }

  std::string host = Conf::database_addr;
  std::string port = "";//Conf::database_port;
  std::string user = Conf::database_user;;
  std::string passwd = Conf::database_passwd;
  std::string dbname = "";//Conf::database_database;

  std::string cs;
  if(host.size()) cs += " host=" + host;
  if(port.size()) cs += " port=" + port;
  if(user.size()) cs += " user=" + user;
  if(passwd.size()) cs += " password=" + passwd;
  cs += " dbname=" + (dbname.size() ? dbname : "pracro");

  pqxx::connection conn(cs);
  pqxx::work work(conn);

  std::set<std::string> filter;

  {
    pqxx::result result =
      work.exec("SELECT DISTINCT name FROM fieldnames"
                " WHERE extract='true';");
    pqxx::result::const_iterator ri = result.begin();
    for(unsigned int r = 0; r < result.size(); r++) {
      pqxx::result::tuple tuple = result.at(r); 
      std::string name = tuple.at(0).c_str();
      filter.insert(name);
      //printf("filter: '%s'\n", name.c_str());
    }
  }

  templates_t t = scanfieldnames(filter);
  /*
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

  {
    pqxx::result result =
      work.exec("SELECT * FROM commits WHERE template LIKE '"+prefix+"%'"
                " AND status='committed' ORDER BY patientid, timestamp;");
    pqxx::result::const_iterator ri = result.begin();
    for(unsigned int r = 0; r < result.size(); r++) {
      pqxx::result::tuple tuple = result.at(r); 
      std::string patientid = tuple.at(0).c_str();
      std::string templ = tuple.at(1).c_str();
      std::string version = tuple.at(2).c_str();
      std::string timestamp = tuple.at(3).c_str();
      std::string uid = tuple.at(4).c_str();
      std::string status = tuple.at(5).c_str();

      if(files.find(templ) == files.end()) {
        files[templ] = new File(templ, t[templ], work);
      }

      files[templ]->beginrow();
      files[templ]->addcell("id", uid);
      files[templ]->addcell("patientid", patientid);
      files[templ]->addcell("template", templ);
      time_t t = atol(timestamp.c_str());
      std::string timestr = ctime(&t);
      if(timestr[timestr.size() - 1] == '\n')
        timestr = timestr.substr(0, timestr.size() - 1);
      files[templ]->addcell("time", timestr.c_str());

      DEBUG(export, "%s %s %s\n",
            timestamp.c_str(), patientid.c_str(), templ.c_str());

      {
        pqxx::result result =
          work.exec("SELECT f.name, f.value"
                    " FROM transactions t, fields f, fieldnames n"
                    " WHERE t.cid='"+uid+"' AND f.transaction=t.uid"
                    "  AND f.name=n.name AND n.extract='true'"
                    " ORDER BY t.timestamp;");
        pqxx::result::const_iterator ri = result.begin();
        for(unsigned int r = 0; r < result.size(); r++) {
          pqxx::result::tuple tuple = result.at(r); 
          std::string name = tuple.at(0).c_str();
          std::string value = tuple.at(1).c_str();

          files[templ]->addcell(name, value);

        }
      }

      files[templ]->endrow();
    }
  }
  
  std::map<std::string, File *>::iterator i = files.begin();
  while(i != files.end()) {
    delete i->second;
    i++;
  }
}

#endif/* WITHOUT_DB */

static const char usage_str[] =
"  help        Prints this helptext.\n"
"  prefix p    Export all templates matching the prefix p.\n"
;

void macrotool_export(std::vector<std::string> params)
{
  if(params.size() < 1) {
    printf("%s", usage_str);
    return;
  }

  DEBUG(export, "export: %s\n", params[0].c_str());

  if(params[0] == "prefix") {
    if(params.size() != 2) {
      printf("The command 'prefix' needs a parameter.\n");
      printf("%s", usage_str);
      return;
    }
#ifndef WITHOUT_DB
    export_prefix(params[1]);
#endif/* WITHOUT_DB */
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


#ifdef TEST_EXPORT
//Additional dependency files
//deps:
//Required cflags (autoconf vars may be used)
//cflags:
//Required link options (autoconf vars may be used)
//libs:
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).

TEST_END;

#endif/*TEST_EXPORT*/

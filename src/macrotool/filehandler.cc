/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macrotool_filehandler.cc
 *
 *  Fri Jul 17 08:48:09 CEST 2009
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
#include "filehandler.h"

#include <fstream>
#include <ios>

#include <string.h>

#include "macroheaderparser.h"
#include "macroparser.h"
#include "template.h"

#include <hugin.h>

#include "util.h"

#include "configuration.h"

static const char usage_str[] =
"  help           Prints this helptext.\n"
"  add file       Add a file called 'file' to the macro or template folder, according\n"
"                  to its contents.\n"
"  check file     Check if a file is valid, and print a resume of its contents.\n"
;

/**
class Macro {
public:
  std::vector< Query > queries;
  std::vector< Map > maps;
  std::vector< Script > scripts;
  Widget widgets;
  std::map< std::string, std::string > attributes;
  Resume resume;
};
 **/
static bool check(std::string file, std::string *name = NULL, std::string *version = NULL);
static bool check(std::string file, std::string *name, std::string *version)
{
  try {
    MacroHeaderParser parser(file);
    parser.parse();
    Macro *macro = parser.getMacro();

    if(!macro) {
      printf("Macro malformed!\n");
      return false;
    }

    printf("Parsing of %s was succesful.\n", file.c_str());
    printf("Name:    %s\n", macro->name.c_str());
    printf("Version: %s\n", macro->version.c_str());

    if(name) *name = macro->name;
    if(version) *version = macro->version;

  } catch( std::exception &e ) {
    printf("%s\n", e.what());
    return false;
  }
  return true;
}

#define SZ 1 // For stress test purposes set this to a large number (1000)
static bool macro_exists(std::string name, std::string version, std::string &clashfile)
{
  std::vector<std::string> macrofiles = getMacros();

  for(int prut = 0; prut < SZ; prut++) {
  std::vector<std::string>::iterator mfs = macrofiles.begin();
  while(mfs != macrofiles.end()) {
    std::string macroname = mfs->substr(0, mfs->length() - 4);

    MacroHeaderParser parser(Conf::xml_basedir + "/macros/" + *mfs);
    //MacroParser parser(macroname);
    parser.parse();
    Macro *macro = parser.getMacro();

    if(name == macro->name && version == macro->version) {
      clashfile = *mfs;
      return true;
    }

    mfs++;
  }
  }
  printf("Parsed %d files\n", macrofiles.size() * SZ);
  return false;
}

static std::string strippath(std::string filename)
{
  if(filename.find('/') == std::string::npos) return filename;
  return filename.substr(filename.rfind('/')+1);
}

static bool file_exist(std::string file)
{
  FILE *fp =  fopen(file.c_str(), "r");
  if(!fp) return false;
  fclose(fp);
  return true;
}

static void add(std::string file)
{
  std::string name;
  std::string version;
  std::string clashfile;
  std::string target;

  if(!check(file, &name, &version)) {
    printf("File not a valid macro file.\nAborting...\n");
    return;
  }

  if(macro_exists(name, version, clashfile)) {
    printf("WARNING: A macro with that name and version already exists."
           " THE EXISTING FILE WILL BE OVERWRITTEN!\n");
    printf("File: %s\n", clashfile.c_str());
    char answer[32];
    answer[0] = '\0';
    while(std::string(answer) != "yes\n" && std::string(answer) != "no\n") {
      if(answer[0] == '\0') printf("Are you sure you want to put the file in the macro directory? [yes/no]\n");
      else printf("Please answer 'yes' or 'no'\n");
      fgets(answer, sizeof(answer), stdin);
    }
    
    if(std::string(answer) == "no\n") {
      printf("Aborting...\n");
      return;
    }
    target = Conf::xml_basedir + "/macros/" + clashfile;
  } else {
    target = Conf::xml_basedir + "/macros/" + strippath(file);

    size_t cnt = 0;
    while(file_exist(target)) {
      char *num;
      if(cnt) asprintf(&num, "-%d", cnt);
      else num = strdup("");
      target = Conf::xml_basedir + "/macros/" + name + "-" + version + num + ".xml";
      printf("Trying: %d %s\n", cnt, target.c_str());
      free(num);
      cnt++;
    }
  }

  printf("Copying '%s' to '%s' ...\n", file.c_str(), target.c_str());

  {
    std::ifstream ifs(file.c_str(), std::ios::binary);
    if(!ifs) {
      printf("Could read source file.\nAborting...\n");
      return;
    }
    std::ofstream ofs(target.c_str(), std::ios::binary);
    ofs << ifs.rdbuf();
  }
  printf("done\n");
}

void macrotool_filehandler(std::vector<std::string> params)
{
  if(params.size() < 1) {
    printf("%s", usage_str);
    return;
  }

  DEBUG(filehandler, "filehandler: %s\n", params[0].c_str());

  if(params[0] == "add") {
    if(params.size() != 2) {
      printf("The command 'add' needs 1 parameter.\n");
      printf("%s", usage_str);
      return;
    }
    add(params[1]);
    return;
  }

  if(params[0] == "check") {
    if(params.size() != 2) {
      printf("The command 'check' needs 1 parameter.\n");
      printf("%s", usage_str);
      return;
    }
    check(params[1]);
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

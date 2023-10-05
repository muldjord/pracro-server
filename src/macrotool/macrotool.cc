/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macrotool.cc
 *
 *  Mon Jul  6 08:25:28 CEST 2009
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
// For ETC
#include <config.h>

#include <stdlib.h>
#include <string.h>

// For getopt_long and friends
#include <getopt.h>

#include <vector>
#include <string>

#include "configurationparser.h"
#include "configuration.h"

#include <hugin.h>

#include "dump.h"
#include "fieldnames.h"
#include "filehandler.h"
#include "export.h"
#include "sunlock.h"

static const char version_str[] =
"Pracro server v" VERSION "\n"
;

static const char copyright_str[] =
"Copyright (C) 2006-2009 Bent Bisballe Nyeng - Aasimon.org.\n"
"This is free software.  You may redistribute copies of it under the terms of\n"
"the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
"There is NO WARRANTY, to the extent permitted by law.\n"
"\n"
"Written by Bent Bisballe Nyeng (deva@aasimon.org)\n"
;

static const char usage_str[] =
"Usage: %s [options] command\n"
"Options:\n"
"  -c, --config file   Read configfile from 'file'\n"
"  -x, --xml-basedir d Use 'd' as basedir for finding template- and macro-files (default " XML ").\n"
"  -v, --version       Print version information and exit.\n"
"  -h, --help          Print this message and exit.\n"
"  -D, --debug ddd     Enable debug messages on 'ddd'; see documentation for details\n"
"\n"
"Commands:\n"
"  dump entity         Dumps 'entity' to screen ('dump help' to see list of entities).\n"
"  fieldnames entity   Add/delete/update entries in the fieldnames database\n"
"                       ('fieldnames help' to see list of entities).\n"
"  filehandler entity  Handle macro files ('filehandler help' to see list of entities).\n"
"  export entity       Export data from database to comma separated file ('export help' to see list of entities)\n"
"  sunlock sid         Sets the status of a session id in the database"
                       " to 'idle'.\n"
;

ConfigurationParser *configparser = NULL;

int main(int argc, char *argv[])
{
  int c;
  std::string configfile;
  std::string xml_basedir;
  char *debugstr = NULL;

  debug_init(stderr);

  int option_index = 0;
  while(1) {
    static struct option long_options[] = {
      {"config", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {"xml-basedir", required_argument, 0, 'x'},
      {"debug", required_argument, 0, 'D'},
      {0, 0, 0, 0}
    };
    
    c = getopt_long (argc, argv, "D:hvc:x:", long_options, &option_index);
    
    if (c == -1)
      break;

    switch(c) {
    case 'c':
      configfile = optarg;
      break;

    case 'x':
      xml_basedir = optarg;
      break;

    case 'D':
      debugstr = strdup(optarg);
      break;

    case '?':
    case 'h':
      printf("%s", version_str);
      printf(usage_str, argv[0]);
      return 0;

    case 'v':
      printf("%s", version_str);
      printf("%s", copyright_str);
      return 0;

    default:
      break;
    }
  }

  if(optind >= argc) {
    fprintf(stderr, "Missing command\n");
    printf(usage_str, argv[0]);
    exit(EXIT_FAILURE);
  }

  if(debugstr) {
    debug_parse(debugstr);
  }

  // Load config
  if(configfile == "") configfile = ETC"/pracrod.conf";
  configparser = new ConfigurationParser(configfile);

  if(xml_basedir != "") {
    Conf::xml_basedir = xml_basedir;
  }

  std::string command = argv[optind++];
  std::vector<std::string> params;
  while(optind < argc) {
    params.push_back(argv[optind++]);
  }

  if(command == "dump") macrotool_dump(params);
  if(command == "fieldnames") macrotool_fieldnames(params);
  if(command == "filehandler") macrotool_filehandler(params);
  if(command == "export") macrotool_export(params);
  if(command == "sunlock") macrotool_sunlock(params);

  return 0;
}

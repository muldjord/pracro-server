/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configurationparser.cc
 *
 *  Wed Jul 30 11:48:31 CEST 2008
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

#include "configurationparser.h"
#include "configuration.h"

#include <fstream>
#include <sstream>

const char *ConfigurationParser::ParseException::what()
{
  char lineno[32];
  sprintf(lineno, "%d", l);
  _what = "Error when parsing the config file at line ";
  _what += std::string(lineno) + ": " + e;
  return _what.c_str();
}

ConfigurationParser::ConfigurationParser(std::string filename)
{
  this->filename = filename;

  reload();
}

void ConfigurationParser::reload()
{
  try {
    readFile(this->filename.c_str());
  } catch(libconfig::FileIOException) {
    throw ReadException();
  } catch(libconfig::ParseException &e) {
    throw ParseException(e.getLine(), e.getError());
  }

  // Set internal values
  try {
    int p = lookup("server_port");
    Conf::server_port = p;
  } catch( ... ) {
  }

  try {
    std::string u = lookup("server_user");
    Conf::server_user = u;
  } catch( ... ) {
  }

  try {
    std::string g = lookup("server_group");
    Conf::server_group = g;
  } catch( ... ) {
  }

  try {
    std::string p = lookup("server_passwd");
    Conf::server_passwd = p;
  } catch( ... ) {
  }

  try {
    std::string a = lookup("journal_commit_addr");
    Conf::journal_commit_addr = a;
  } catch( ... ) {
  }

  try {
    int p = lookup("journal_commit_port");
    Conf::journal_commit_port = p;
  } catch( ... ) {
  }
  
  try {
    int t = lookup("db_max_ttl");
    Conf::db_max_ttl = t;
  } catch( ... ) {
  }

  try {
    int t = lookup("pentominos_max_ttl");
    Conf::pentominos_max_ttl = t;
  } catch( ... ) {
  }

  try {
    std::string s = lookup("artefact_addr");
    Conf::artefact_addr = s;
  } catch( ... ) {
  }

  try {
    int i = lookup("artefact_port");
    Conf::artefact_port = i;
  } catch( ... ) {
  }

  try {
    bool b = lookup("artefact_use_ssl");
    Conf::artefact_use_ssl = b;
  } catch( ... ) {
  }

  try {
    int i = lookup("artefact_poolsize");
    Conf::artefact_poolsize = i;
  } catch( ... ) {
  }

  try {
    std::string a = lookup("database_backend");
    Conf::database_backend = a;
  } catch( ... ) {
  }

  try {
    std::string a = lookup("database_addr");
    Conf::database_addr = a;
  } catch( ... ) {
  }

  try {
    std::string u = lookup("database_user");
    Conf::database_user = u;
  } catch( ... ) {
  }

  try {
    std::string p = lookup("database_passwd");
    Conf::database_passwd = p;
  } catch( ... ) {
  }

  try {
    int i = lookup("database_poolsize");
    Conf::database_poolsize = i;
  } catch( ... ) {
  }

  try {
    std::string p = lookup("xml_basedir");
    Conf::xml_basedir = p;
  } catch( ... ) {
  }

  try {
    bool b = lookup("use_ssl");
    Conf::use_ssl = b;
  } catch( ... ) {
  }

  try {
    std::string s = lookup("ssl_key");
    std::ifstream f(s);
    if(f.is_open()) {
      std::stringstream buffer;
      buffer << f.rdbuf();
      f.close();
      s = buffer.str();
    }
    Conf::ssl_key = s;
  } catch( ... ) {
  }

  try {
    std::string s = lookup("ssl_cert");
    std::ifstream f(s);
    if(f.is_open()) {
      std::stringstream buffer;
      buffer << f.rdbuf();
      f.close();
      s = buffer.str();
    }
    Conf::ssl_cert = s;
  } catch( ... ) {
  }

  try {
    int i = lookup("connection_limit");
    Conf::connection_limit = i;
  } catch( ... ) {
  }

  try {
    int i = lookup("connection_timeout");
    Conf::connection_timeout = i;
  } catch( ... ) {
  }

  try {
    std::string s = lookup("session_path");
    Conf::session_path = s;
  } catch( ... ) {
  }

  try {
    std::string s = lookup("session_discard_path");
    Conf::session_discard_path = s;
  } catch( ... ) {
  }
}

#ifdef TEST_CONFIGURATIONPARSER
//deps: configuration.cc
//cflags: -I.. $(CONFIG_CXXFLAGS)
//libs: $(CONFIG_LIBS)
#include <test.h>

#define CONFFILE "/tmp/configurationparser.conf"
#define NOSUCH_CONFFILE "/tmp/ladida_configurationparser.conf"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

static char conf[] = 
"a = 1;\n"
"b = \"hello\";\n"
"c = true;\n"
;

static char confbad[] = 
"a = 1;\n"
"b = \"hello\"\n"
"c = true;\n"
;

TEST_BEGIN;

FILE *fp = fopen(CONFFILE, "w");
if(!fp) TEST_FATAL("Could not write to "CONFFILE"\n");
fprintf(fp, "%s", conf);
fclose(fp);

TEST_NOEXCEPTION(ConfigurationParser parser(CONFFILE), "Creation");

fp = fopen(CONFFILE, "w");
if(!fp) TEST_FATAL("Could not write to "CONFFILE"\n");
fprintf(fp, "%s", confbad);
fclose(fp);

TEST_EXCEPTION(ConfigurationParser parser(CONFFILE), 
               ConfigurationParser::ParseException, "Bad syntax");

TEST_EXCEPTION(ConfigurationParser parser(NOSUCH_CONFFILE), 
               ConfigurationParser::ReadException, "No such file");

unlink(CONFFILE);

TEST_END;

#endif/*TEST_CONFIGURATIONPARSER*/

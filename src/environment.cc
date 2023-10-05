/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            environment.cc
 *
 *  Tue Jan  5 11:41:23 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#include "environment.h"

#include "configuration.h"
#include "database.h"

Environment::Environment()
  : sessions(this),
    macrolist(Conf::xml_basedir + "/macros"),
    templatelist(Conf::xml_basedir + "/templates"),
    courselist(Conf::xml_basedir + "/courses")
{

  for(int i = 0; i < Conf::database_poolsize; i++) {
    dbpool.add(new Database(Conf::database_backend, Conf::database_addr,
                          "", Conf::database_user, Conf::database_passwd, ""));
  }

  for(int i = 0; i < Conf::artefact_poolsize; i++) {
    atfpool.add(new Artefact);
  }
}

Environment::~Environment()
{
  /*
  // Remove, but wait until resources are released
  std::list<Database*> dblst = dbpool.clear(false);
  std::list<Database*>::iterator i = dblst.begin();
  while(i != dblst.end()) {
    delete *i;
    i++;
  }
  */
  // Remove, but wait until resources are released
  std::list<Artefact*> atflst = atfpool.clear(false);
  std::list<Artefact*>::iterator j = atflst.begin();
  while(j != atflst.end()) {
    delete *j;
    j++;
  }
}

#ifdef TEST_ENVIRONMENT
//deps: configuration.cc database.cc artefact.cc pracrodao.cc session.cc mutex.cc semaphore.cc debug.cc pracrodaotest.cc pracrodaopgsql.cc journal.cc journal_commit.cc entitylist.cc inotify.cc exception.cc versionstr.cc tcpsocket.cc macrolist.cc templatelist.cc saxparser.cc log.cc macroheaderparser.cc templateheaderparser.cc sessionserialiser.cc journal_uploadserver.cc xml_encode_decode.cc sessionparser.cc sessionheaderparser.cc luascript.cc courselist.cc luautil.cc courseparser.cc
//cflags: -DWITHOUT_ARTEFACT -I.. $(PQXX_CXXFLAGS) $(PTHREAD_CFLAGS) $(EXPAT_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(PQXX_LIBS) -lpthread $(EXPAT_LIBS) $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include "test.h"

TEST_BEGIN;

Conf::database_backend = "testdb";
Conf::database_poolsize = 1;

Conf::artefact_poolsize = 1;

Conf::xml_basedir = "/tmp";

TEST_NOEXCEPTION(Environment env, "Check if the Enviroment can be created.");

TEST_END;

#endif/*TEST_ENVIRONMENT*/

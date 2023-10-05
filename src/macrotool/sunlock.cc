/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sunlock.cc
 *
 *  Wed Feb  2 10:28:14 CET 2011
 *  Copyright 2011 Lars Bisballe Jensen
 *  elsenator@gmail.com
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
#include "sunlock.h"

#include <config.h>
#include <stdio.h>

#ifndef WITHOUT_DB

#include <pqxx/pqxx>

#include "configuration.h"

static void sunlock_uid(std::string uid)
{
  if(Conf::database_backend != "pgsql") {
    printf("ERROR: Sunlock only available for the pgsql database backend.\n");
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

  work.exec("UPDATE COMMITS SET status='idle' WHERE uid="+uid+" AND status='active';");
  work.commit();
}

#endif/* WITHOUT_DB */

static const char usage_str[] =
"  help        Prints this helptext.\n"
"  sid         Sets the status of a session id in the database"
               " to 'idle'.\n"
;

void macrotool_sunlock(std::vector<std::string> params)
{
  if(params.size() < 1) {
    printf("%s", usage_str);
    return;
  }

  if(params[0] == "help") {
    printf("%s", usage_str);
    return;
  }
  
#ifndef WITHOUT_DB
  sunlock_uid(params[0]);
#endif/* WITHOUT_DB */
}

#ifdef TEST_SUNLOCK
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

#endif/*TEST_SUNLOCK*/

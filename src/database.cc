/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            database.cc
 *
 *  Thu Sep  6 10:59:07 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
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
#include "database.h"

#include <config.h>
#include <stdlib.h>

#include <hugin.hpp>
#include "pracrodaopgsql.h"
#include "pracrodaotest.h"

Database::Database(std::string _backend, std::string _host, std::string _port,
                   std::string _user, std::string _passwd, std::string _dbname)
{
  dao = nullptr;
#ifndef WITHOUT_DB
  if(_backend == "pgsql") {
    DEBUG(db, "construct(%s, %s, %s, %s, %s)\n",\
          _host.c_str(), _port.c_str(), _user.c_str(),
          _passwd.c_str(), _dbname.c_str());
    dao = new PracroDAOPgsql(_host, _port, _user, _passwd, _dbname);
    return;
  }
#endif/*WITHOUT_DB*/
  if(_backend == "testdb") {
    DEBUG(db, "Running with 'testdb'\n");
    dao = new PracroDAOTest(true);
    return;
  }

  ERR(db, "Cannot find database backend \"%s\"", _backend.c_str());
}

Database::~Database()
{
#ifndef WITHOUT_DB
  if(dao) delete dao;
#endif/*WITHOUT_DB*/
}


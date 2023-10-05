/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryhandlerpentominos.cc
 *
 *  Thu Jan 15 11:35:31 CET 2009
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
#include "queryhandlerpentominos.h"

#include <hugin.hpp>

#include <config.h>

QueryHandlerPentominos::QueryHandlerPentominos(Artefact &atf,
                                               std::string patientid,
                                               std::string user)
  : QueryHandler(), artefact(atf)
{
  this->patientid = patientid;
  this->user = user;
}

QueryResult QueryHandlerPentominos::exec(Query &query)
{
  return artefact.exec(query, patientid, user);
}

#ifdef TEST_QUERYHANDLERPENTOMINOS
//deps: artefact.cc configuration.cc debug.cc log.cc mutex.cc
//cflags: -I.. $(ATF_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(ATF_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#include "tcpsocket.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_QUERYHANDLERPENTOMINOS*/

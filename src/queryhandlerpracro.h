/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryhandlerpracro.h
 *
 *  Thu Jan 15 11:35:34 CET 2009
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
#ifndef __PRACRO_QUERYHANDLERPRACRO_H__
#define __PRACRO_QUERYHANDLERPRACRO_H__

#include "queryhandler.h"

#include "database.h"
#include "template.h"
#include "queryresult.h"

#include <vector>
#include <string>

/**
 * This class handles the query of pracro data.
 */
class QueryHandlerPracro : public QueryHandler {
public:
  QueryHandlerPracro(Database &db, std::string cpr, std::string sessionid);
  ~QueryHandlerPracro() {}

  // Execute all queries.
  QueryResult exec(Query &query);

private:
  Database &db;
  std::string cpr;
  std::string sessionid;
};

#endif/*__PRACRO_QUERYHANDLERPRACRO_H__*/

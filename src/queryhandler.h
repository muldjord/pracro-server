/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryhandler.h
 *
 *  Tue May  6 14:50:55 CEST 2008
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
#ifndef __PRACRO_QUERYHANDLER_H__
#define __PRACRO_QUERYHANDLER_H__

#include "template.h"
#include "queryresult.h"

#include <vector>
#include <string>

/**
 * This class handles the query of external data.
 */
class QueryHandler {
public:
  QueryHandler() {}
  virtual ~QueryHandler() {}

  // Execute all queries.
  virtual QueryResult exec(Query &query) = 0;
};

#endif/*__PRACRO_QUERYHANDLER_H__*/

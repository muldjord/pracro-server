/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            pgwork.h
 *
 *  Wed Feb 26 14:49:37 CET 2014
 *  Copyright 2014 Bent Bisballe Nyeng
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
#ifndef __PRACRO_PGWORK_H__
#define __PRACRO_PGWORK_H__

#ifndef WITHOUT_DB

#include <list>
#include <vector>
#include <string>

#include <libpq-fe.h>

typedef std::list<std::vector<std::string> > result_t;
typedef std::vector<std::string> tuple_t;

class Work {
public:
  typedef enum {
    WORK_INVALID,
    WORK_OPEN,
    WORK_COMMITTED,
    WORK_ABORTED
  } work_state_t;

  Work(PGconn *_c);
  ~Work(); 
  result_t exec(const std::string &query);
  void commit();
  void abort();
  std::string esc(const std::string &from);

private:
  PGconn *c;
  work_state_t state;
};

#endif/*WITHOUT_DB*/

#endif/*__PRACRO_PGWORK_H__*/

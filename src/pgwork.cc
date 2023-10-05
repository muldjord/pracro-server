/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            pgwork.cc
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
#include "pgwork.h"

#include <stdlib.h>

#include <hugin.hpp>

/**
 * Convert list of tuples to list of string arrays.
 */
static result_t toArray(PGresult *pg)
{
  std::list<std::vector<std::string> > res;
  for(int j = 0; j < PQntuples(pg); j++) {
    std::vector<std::string> v;
    for(int i = 0; i < PQnfields(pg); i++) {
      v.push_back(std::string(PQgetvalue(pg, j, i)));
    }
    res.push_back(v);
  }
  return res;
}

class pq_exc : public std::exception {
public:
  pq_exc(std::string _what) : w(_what) {}
  ~pq_exc() {}
  const char *what() { return w.c_str(); }
private:
  std::string w;
};

Work::Work(PGconn *_c)
  : c(_c)
{
  state = WORK_OPEN;
  try {
    exec("BEGIN");
  } catch(std::exception &e) {
    ERR(db, "Error beginning transaction: %s\n", e.what());
    state = WORK_INVALID;
    throw e;
  }
}

Work::~Work()
{
  if(state == WORK_OPEN) abort();
}
 
/**
 * Execute query and return result as list of string arrays.
 */
result_t Work::exec(const std::string &query)
{
  if(state != WORK_OPEN) {
    ERR(db, "Transaction is not is not valid. Cannot execute query!\n");
    return result_t();
  }
  
  PGresult *pg = PQexec(c, query.c_str());
  if(PQresultStatus(pg) != PGRES_TUPLES_OK &&
     PQresultStatus(pg) != PGRES_COMMAND_OK) {
    throw pq_exc(PQerrorMessage(c));
  }
  
  result_t R = toArray(pg);
  
  PQclear(pg);
  return R;
}

void Work::commit()
{
  try {
    exec("COMMIT");
  } catch(std::exception &e) {
    ERR(db, "Error committing transaction: %s\n", e.what());
    throw e;
  }
  state = WORK_COMMITTED;
}

void Work::abort()
{
  try {
    exec("ROLLBACK");
  } catch(std::exception &e) {
    ERR(db, "Error aborting transaction: %s\n", e.what());
    throw e;
  }
  state = WORK_ABORTED;
}

std::string Work::esc(const std::string &from)
{
  if(from == "") return "";
  
  size_t buf_size = from.length() * 2 + 1;
  char *buf = (char *)malloc(buf_size);
  int error = 0;
  
  size_t size = PQescapeStringConn(c, buf, from.data(),
                                   from.length(), &error);
  
  if(error) {
    ERR(db, "SQL escaping failed due to invalid multibyte character"
        " in '%s'\n", from.c_str());
    return "";
  }
  
  std::string to;
  if(size > 0 && error == 0) to.append(buf, size);
  
  free(buf);
  return to;
}

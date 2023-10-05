/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            pracrodaotest.h
 *
 *  Fri Aug  7 10:25:07 CEST 2009
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
#ifndef __PRACRO_PRACRODAOTEST_H__
#define __PRACRO_PRACRODAOTEST_H__

#include <string>
#include <vector>
#include <map>

#include <config.h>
#include <stdio.h>

#include "pracrodao.h"

typedef std::map<std::string, std::string> dbrow_t; 
typedef std::vector< dbrow_t > dbtable_t;

class dbcounter_t {
public:
  dbcounter_t()
  {
    counter = 0;
  }

  std::string nextval()
  {
    counter++;
    return currval();
  }

  std::string currval()
  {
    char buf[32];
    sprintf(buf, "%d", (int)counter);
    return buf;
  }

private:
  size_t counter;
};

class Data {
public:
  dbcounter_t sessionseq;
  dbcounter_t trseq;
  dbtable_t transactions;
  dbtable_t fieldnames;
  dbtable_t fields;
  dbtable_t commits;
};

class PracroDAOTest : public PracroDAO
{
public:
  PracroDAOTest(bool ignore_fieldnames = false);
  ~PracroDAOTest();

  std::string newSessionId();
  void commitTransaction(std::string sessionid,
                         Transaction &transaction,
                         Commit &commit,
                         Macro &macro,
                         time_t now);
  Values getLatestValues(std::string sessionid,
                         std::string patientid,
                         Macro *macro,
                         Fieldnames &fieldnames,
                         time_t oldest);
  unsigned nrOfCommits(std::string sessionid,
                       std::string patientid,
                       std::string macroname,
                       time_t oldest);

  void addFieldname(std::string name, std::string description);
  void delFieldname(std::string name);
  std::vector<Fieldname> getFieldnames();

  void commit(std::string sessionid);
  void nocommit(std::string sessionid);
  void discard(std::string sessionid);

  std::string serialise() { return ""; }
  void restore(const std::string &data) {}

  bool idle(std::string sessionid);
  void setIdle(std::string sessionid, bool idle);

private: 
  Data data;
  bool ignore_fieldnames;
};

#endif/*__PRACRO_PRACRODAOTEST_H__*/

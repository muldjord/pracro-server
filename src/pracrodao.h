/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            pracrodao.h
 *
 *  Wed Feb 11 11:17:37 CET 2009
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
#ifndef __PRACRO_PRACRODAO_H__
#define __PRACRO_PRACRODAO_H__

#include <string>
#include <vector>

#include "dbtypes.h"
#include "template.h"
#include "transaction.h"

class PracroDAO
{
public:
  PracroDAO(std::string _host, std::string _port,
            std::string _user, std::string _passwd, std::string _dbname);
  virtual ~PracroDAO();

  virtual std::string newSessionId() = 0;
  virtual void commitTransaction(std::string sessionid,
                                 Transaction &transaction,
                                 Commit &commit,
                                 Macro &macro,
                                 time_t now) = 0;
  virtual Values getLatestValues(std::string sessionid,
                                 std::string patientid,
                                 Macro *macro,
                                 Fieldnames &fieldnames,
                                 time_t oldest) = 0;
  virtual unsigned nrOfCommits(std::string sessionid,
                               std::string patientid,
                               std::string macroname,
                               time_t oldest) = 0;

  virtual void addFieldname(std::string name,
                            std::string description) = 0;
  virtual void delFieldname(std::string name) = 0;
  virtual std::vector<Fieldname> getFieldnames() = 0;

  virtual void commit(std::string sessionid) = 0;
  virtual void nocommit(std::string sessionid) = 0;
  virtual void discard(std::string sessionid) = 0;

  virtual std::string serialise() = 0;
  virtual void restore(const std::string &data) = 0;

  virtual bool idle(std::string sessionid) = 0;
  virtual void setIdle(std::string sessionid, bool idle) = 0;

protected:
  std::string host;
  std::string port;
  std::string user;
  std::string passwd;
  std::string dbname;
};

#endif/*__PRACRO_PRACRODAO_H__*/

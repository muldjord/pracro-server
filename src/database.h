/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            database.h
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
#ifndef __PRACRO_DATABASE_H__
#define __PRACRO_DATABASE_H__

#include <time.h>
#include "pracrodao.h"
#include "transaction.h"
#include "template.h"
#include "mutex.h"
#include <hugin.hpp>

class Database {
public:
  Database(std::string _backend, std::string _host,
           std::string _port, std::string _user,
           std::string _passwd, std::string _dbname);
  ~Database();

  std::string newSessionId()
  {
    if(dao) {
      return dao->newSessionId();
    }
    return "";
  }
  /*
  void setSessionId(std::string sessionid)
  {
    this->sessionid = sessionid;
  }
  */
  // Make a commit to the db
  void commitTransaction(Transaction &transaction,
                         Commit &commit,
                         Macro &macro,
                         std::string sessionid,
                         time_t now = time(nullptr))
  {
    if(!dao) return;

    mutex.lock();
    DEBUG(db, "%s, %s, %s,...\n",
          transaction.user.c_str(), transaction.patientid.c_str(),
          macro.name.c_str());
    dao->commitTransaction(sessionid, transaction, commit, macro, now);
    mutex.unlock();
  }

  // Get a list of values from the db
  Values getValues(std::string patientid,
                   Fieldnames &fieldnames,
                   std::string sessionid,
                   time_t oldest)
  {
    if(!dao) return Values();
    mutex.lock();
    DEBUG(db, "%s, <%u fieldnames>, %ld\n",
          patientid.c_str(), (int)fieldnames.size(), oldest);
    Values values = dao->getLatestValues(sessionid, patientid,
                                         nullptr, fieldnames, oldest);
    mutex.unlock();
    return values;
  }

  // Check if a macro has been committed.
  bool checkMacro(std::string patientid,
                  std::string macro,
                  std::string sessionid,
                  time_t oldest)
  {
    DEBUG(db, "%s, %s, %ld\n",
          patientid.c_str(), macro.c_str(), oldest);
    if(!dao) return false;
    mutex.lock();
    bool res = dao->nrOfCommits(sessionid, patientid, macro, oldest) > 0;
    mutex.unlock();
    return res;
  }
  
  // Get latest resume of a given macro
  std::string getResume(std::string patientid, Macro &macro,
                        time_t oldest, std::string sessionid)
  {
    DEBUG(db, "%s, %s, %ld\n",
          patientid.c_str(), macro.name.c_str(), oldest);
    if(!dao) return "";
    Fieldnames fn;
    fn.push_back("journal.resume");
    mutex.lock();
    Values v = dao->getLatestValues(sessionid, patientid, &macro, fn, oldest);
    mutex.unlock();
    Values::iterator i = v.find("journal.resume");
    if(i != v.end())  return i->second.value;
    else return "";
  }

  void addFieldname(std::string name, std::string description)
  {
    if(!dao) return;
    mutex.lock();
    dao->addFieldname(name, description);
    mutex.unlock();
  }

  void delFieldname(std::string name)
  {
    if(!dao) return;
    mutex.lock();
    dao->delFieldname(name);
    mutex.unlock();
  }
  
  std::vector<Fieldname> getFieldnames()
  {
    if(!dao) {
      std::vector<Fieldname> fieldnames;
      return fieldnames;
    }
    mutex.lock();
    std::vector<Fieldname> fieldnames = dao->getFieldnames(); 
    mutex.unlock();
    return fieldnames;
  }

  void commit(std::string sessionid)
  {
    if(!dao || sessionid == "") return;
    return dao->commit(sessionid);
  }

  void nocommit(std::string sessionid)
  {
    if(!dao || sessionid == "") return;
    return dao->nocommit(sessionid);
  }

  void discard(std::string sessionid)
  {
    if(!dao || sessionid == "") return;
    return dao->discard(sessionid);
  }

  std::string serialise()
  {
    if(!dao) return "";
    return dao->serialise();
  }

  void restore(const std::string &data)
  {
    if(!dao) return;
    return dao->restore(data);
  }

  bool idle(std::string sessionid)
  {
    if(!dao || sessionid == "") return false;
    return dao->idle(sessionid);
  }

  void setIdle(std::string sessionid, bool val)
  {
    if(dao && sessionid != "") {
      dao->setIdle(sessionid, val);
    }
  }

private:
  PracroDAO *dao;
  Mutex mutex;
};

#endif/*__PRACRO_DATABASE_H__*/

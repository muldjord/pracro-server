/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            journal.h
 *
 *  Mon Jun 21 12:42:15 CEST 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#ifndef __PRACRO_JOURNAL_H__
#define __PRACRO_JOURNAL_H__

#include <string>
#include <map>

#include "transaction.h"
#include "template.h"
#include "luaoncommit.h"

class SessionSerialiser;

class Journal {
  friend class SessionSerialiser;
public:
  class Exception {
  public:
    Exception(std::string m) : msg(m) {}
    std::string msg;
  };

  Journal();
  virtual ~Journal() {};

  void addEntry(Transaction &transaction, Commit &commit,
                std::string resume, Template *templ, LUAOnCommit *oncommit);

  void addEntry(std::string resume, std::string macro,
                std::string user, int index, LUAOnCommit *oncommit);

  void runOnCommitScripts();

  virtual void commit() = 0;

  std::string getEntry(std::string macro);
  void removeEntry(std::string macro);

  void setDirty(std::string macro);
  bool dirty(std::string macro);
  
  void setPatientID(std::string id);
  std::string patientID();

protected:
  class ResumeEntry {
  public:
    std::string resume;
    std::string macro;
    std::string user;
    LUAOnCommit *oncommit;
    bool dirty;
  };

  std::string _patientid;

  std::map< int, ResumeEntry > entrylist;
};

#endif/*__PRACRO_JOURNAL_H__*/

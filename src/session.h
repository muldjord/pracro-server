/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            session.h
 *
 *  Tue Dec 15 13:36:49 CET 2009
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
#ifndef __PRACRO_SESSION_H__
#define __PRACRO_SESSION_H__

#include <string>
#include <map>
#include <exception>
#include <vector>

#include "mutex.h"
#include "transaction.h"
#include "template.h"

#include "luascript.h"
#include "journal.h"

class Environment;
class Journal;

class Session {
  friend class SessionSerialiser;
public:
  Session(Environment *env,
          std::string sessionid, std::string patientid, std::string templ);
  ~Session();

  std::string id();
  
  void lock();
  bool trylock();
  void unlock();

  void commit();
  void nocommit();
  void discard();

  void commitMacro(Transaction &transaction, Commit &commit, Macro &macro);

  Journal *journal();

  std::string patientid;
  std::string templ;

  bool idle();
  void setIdle(bool idle);

  bool isReadonly() { return isreadonly; }

private:
  Environment *env;
  Journal *_journal;
  std::string sessionid;
  bool isreadonly;
#ifdef TEST_SESSION
public:
#endif
  Mutex mutex;

};

class Sessions {
public:
  class SessionAlreadyActive : public std::exception {
  public:
    SessionAlreadyActive(const std::string sid) : sessionid(sid) {}
    ~SessionAlreadyActive() {}
    const std::string sessionid;
  };

  Sessions(Environment *env);

  /**
   * Create a new session, with a unique id. Insert it into the session list,
   * and return its pointer.
   */
  Session *newLockedSession(std::string patientid, std::string templ);

  /**
   * Lookup session in session list. Returns the session or nullptr if no session
   * exists with that sessionid.
   */
  Session *lockedSession(std::string sessionid);

  /**
   * Remove session from the session list and return its pointer. It is up to
   * the caller to delete it.
   */
  Session *takeSession(std::string sessionid);

  /**
   * Remove session from the session list and delete it.
   */
  void deleteSession(std::string sessionid);

  /**
   * Return number of active sessions.
   */
  size_t size();

  /**
   * Write all active sessions to disc.
   */
  void store();

  //
  // Admin methods
  //
  class SessionInfo {
  public:
    std::string id;
    std::string patientid;
    std::string user;
    std::string course;
    std::string templ;
    bool idle;
    bool ondisc;
  };
  std::vector<SessionInfo> activeSessions();

private:
  std::map<std::string, Session *> sessions;
  Environment *env;
  Mutex mutex;
};

class SessionAutounlock {
public:
  SessionAutounlock(Session **session);
  ~SessionAutounlock();

private:
  Session **session;
};

#endif/*__PRACRO_SESSION_H__*/

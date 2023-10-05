/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            session.cc
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
#include "session.h"

#include <stdlib.h>
#include <stdio.h>

// for stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "journal.h"
#include "journal_uploadserver.h"

#include "database.h"
#include "configuration.h"
#include "connectionpool.h"
#include "sessionserialiser.h"
#include "environment.h"

Session::Session(Environment *e,
                 std::string sid, std::string pid, std::string t)
 : env(e)
{
  _journal = nullptr;

  sessionid = sid;
  patientid = pid;
  templ = t;
  
  mutex.name = "session-" + sid;

  DEBUG(session, "[%p] new Session(sessionid: '%s', patientid: '%s',"
        " template: '%s')\n", this, sid.c_str(), pid.c_str(), t.c_str());

  isreadonly = true;
}

Session::~Session()
{
  DEBUG(session, "[%p] delete Session(sessionid: '%s')\n", this,
        sessionid.c_str());
  if(_journal) delete _journal;
}

std::string Session::id()
{
  if(sessionid == "") {
    AutoBorrower<Database*> borrower(env->dbpool);
    Database *db = borrower.get();
    sessionid = db->newSessionId();
  }

  return sessionid;
}

void Session::lock()
{
  mutex.lock();
  DEBUG(session, "lock() %p (%s)\n", this, sessionid.c_str());
}

bool Session::trylock()
{
  bool r = mutex.trylock();
  DEBUG(session, "trylock() %p (%s) == %s\n",
        this, sessionid.c_str(), r?"true":"false");
  return r;
}

void Session::unlock()
{
  DEBUG(session, "unlock() %p (%s)\n", this, sessionid.c_str());
  mutex.unlock();
}

void Session::commitMacro(Transaction &transaction, Commit &commit,
                          Macro &macro)
{
  DEBUG(session, "[%p] commitMacro(sessionid: '%s')\n", this,
        sessionid.c_str());

  AutoBorrower<Database*> borrower(env->dbpool);
  Database *db = borrower.get();
  db->commitTransaction(transaction, commit, macro, id());
  isreadonly = false;
}

bool Session::idle()
{
  if(isreadonly) return false;

  {
    AutoBorrower<Database*> borrower(env->dbpool);
    Database *db = borrower.get();
    return db->idle(id());
  }
}

void Session::setIdle(bool idle)
{
  if(isreadonly == false) {
    AutoBorrower<Database*> borrower(env->dbpool);
    Database *db = borrower.get();
    return db->setIdle(id(), idle);
  }
}

void Session::commit()
{
  DEBUG(session, "[%p] commit(sessionid: '%s')\n", this, sessionid.c_str());

  if(_journal != nullptr) {
    try {
      _journal->runOnCommitScripts();
    } catch(LUAScript::Exception &e) {
      throw e;
    }
    try {
      _journal->commit();
    } catch(Journal::Exception &e) {
      throw e;
    }
    delete _journal;
    _journal = nullptr;
  }
  if(isreadonly == false) {
    AutoBorrower<Database*> borrower(env->dbpool);
    Database *db = borrower.get();
    db->commit(id());
  }
}

void Session::nocommit()
{
  DEBUG(session, "[%p] nocommit(sessionid: '%s')\n", this, sessionid.c_str());

  if(isreadonly == false) {
    AutoBorrower<Database*> borrower(env->dbpool);
    Database *db = borrower.get();
    db->nocommit(id());
  }
}

void Session::discard()
{
  DEBUG(session, "[%p] discard(sessionid: '%s')\n", this, sessionid.c_str());

  // Store session file in discarded folder before deleting.
  if(isreadonly == false && Conf::session_discard_path != "") {
    SessionSerialiser ser(env, Conf::session_discard_path);
    ser.save(this);
  }

  if(_journal) {
    delete _journal;
    _journal = nullptr;
  }

  if(isreadonly == false) {
    AutoBorrower<Database*> borrower(env->dbpool);
    Database *db = borrower.get();
    db->discard(id());
  }
}

Journal *Session::journal()
{
  if(_journal == nullptr) {
   _journal =
      new JournalUploadServer(Conf::journal_commit_addr,
                              Conf::journal_commit_port);
  }
  return _journal;
}

Sessions::Sessions(Environment *e) : env(e)
{
}

static bool fexists(const std::string &f)
{
  bool ret;

  FILE *fp = fopen(f.c_str(), "r");
  ret = fp != nullptr;
  if(fp) fclose(fp);

  return ret;
}

Session *Sessions::newLockedSession(std::string patientid, std::string templ)
{
  MutexAutolock lock(mutex);

  std::map<std::string, Session *>::iterator i = sessions.begin();
  while(i != sessions.end()) {
    if(i->second->patientid == patientid &&
       i->second->templ == templ) {
      Session *session = i->second;
      if(!session->idle()) {
        DEBUG(session, "Patient/template matched session is already active.");
        throw SessionAlreadyActive(session->id());
      }
      session->lock();
      return session;
    }

    i++;
  }

  { // Look up patientid / template tupple in session files.
    SessionSerialiser ser(env, Conf::session_path);
    Session *session = ser.findFromTupple(patientid, templ);
    if(session != nullptr) {
      sessions[session->id()] = session;
      if(!session->idle()) {
        DEBUG(session, "Looked up session by id is already active.");
        throw SessionAlreadyActive(session->id());
      }
      session->lock();
      return session;
    }
  }

  Session *session = new Session(env, "", patientid, templ);
  sessions[session->id()] = session;
  session->lock();
  return session;
}

Session *Sessions::lockedSession(std::string sessionid)
{
  MutexAutolock lock(mutex);

  if(sessions.find(sessionid) != sessions.end()) {
    Session *s = sessions[sessionid];
    s->lock();
    return s;
  }

  std::string filename = getSessionFilename(Conf::session_path, sessionid);
  if(fexists(filename)) {
    SessionSerialiser ser(env, Conf::session_path);
    Session *s = ser.load(sessionid);
    sessions[s->id()] = s;
    s->lock();
    return s;
  }

  return nullptr;
}

Session *Sessions::takeSession(std::string sessionid)
{
  MutexAutolock lock(mutex);

  DEBUG(session,"%s\n", sessionid.c_str());

  Session *s = nullptr;
  if(sessions.find(sessionid) != sessions.end()) {
    s = sessions[sessionid];
  }

  if(s) {
    sessions.erase(sessionid);
  }
  else DEBUG(session, "No such session!\n");

  return s;
}

void Sessions::deleteSession(std::string sessionid)
{
  Session *s = takeSession(sessionid);
  if(s) delete s;
}

size_t Sessions::size()
{
  return sessions.size();
}

void Sessions::store()
{
  Session *session = nullptr;
  std::string sessionid;
  do {
    bool waitcont = false;
    session = nullptr;
    {
      MutexAutolock lock(mutex);
      std::map<std::string, Session*>::iterator head = sessions.begin();
      if(head != sessions.end()) {
        session = head->second;
        sessionid = head->first;
        if(session->trylock()) {
          sessions.erase(sessionid);
        } else {
          waitcont = true;
        }
      }
    }

    if(waitcont) {
      usleep(200000); // sleep 200ms
      continue;
    }

    if(session != nullptr) {
      SessionSerialiser ser(env, Conf::session_path);
      ser.save(session);
      delete session;
    }
  } while(session != nullptr);

  /*
  MutexAutolock lock(mutex);

  std::map<std::string, Session*>::iterator i = sessions.begin();
  while(i != sessions.end()) {
    SessionSerialiser ser(env, Conf::session_path);
    Session *s = i->second;
    s->lock();
    ser.save(s);
    delete s;
    i++;
  }
  sessions.clear();
  */
}

std::vector<Sessions::SessionInfo> Sessions::activeSessions()
{
  MutexAutolock lock(mutex);

  std::vector<SessionInfo> act;

  std::map<std::string, Session*>::iterator i = sessions.begin();
  while(i != sessions.end()) {
    Session *s = i->second;
    SessionInfo si;
    si.id = i->first;
    si.templ = "LOCKED";

    if(s->trylock()) {
      // si.user = "simpson";
      // si.course = s->course;
      si.patientid = s->patientid;
      si.templ = s->templ;
      si.idle = s->idle();
      // si.ondisc = false;
      s->unlock();
    }
    act.push_back(si);
    i++;
  }
  
  return act;
}

SessionAutounlock::SessionAutounlock(Session **s)
  : session(s)
{
  // session->lock();
}

SessionAutounlock::~SessionAutounlock()
{
  DEBUG(session, "SessionAutounlock(%p)\n", *session);
  if(*session) (*session)->unlock();
}

#ifdef TEST_SESSION
//deps: configuration.cc journal.cc journal_uploadserver.cc journal_commit.cc mutex.cc debug.cc sessionserialiser.cc sessionparser.cc saxparser.cc environment.cc semaphore.cc artefact.cc xml_encode_decode.cc database.cc pracrodaopgsql.cc pracrodaotest.cc pracrodao.cc entitylist.cc macrolist.cc templatelist.cc macroheaderparser.cc templateheaderparser.cc versionstr.cc exception.cc log.cc inotify.cc sessionheaderparser.cc luascript.cc luautil.cc courselist.cc courseparser.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(EXPAT_CFLAGS) -DWITHOUT_ARTEFACT -DWITHOUT_DB $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(PTHREAD_LIBS) $(EXPAT_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include <test.h>

#define PID "0000000000"
#define TMPL "test"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
debug_parse("+all");

Conf::database_backend = "testdb";
Conf::database_poolsize = 1;
Conf::xml_basedir = "../xml";

Environment env;

// Make sure we start out on an empty session directory.
Conf::session_path = "/tmp/test_session";
while(mkdir(Conf::session_path.c_str(), 0777) == -1 && errno == EEXIST) {
  Conf::session_path += "X";
}

Session *s1 = env.sessions.newSession(PID, TMPL);

TEST_EXCEPTION(env.sessions.newSession(PID, TMPL),
               Sessions::SessionAlreadyActive, "Session should be 'locked'");

Session *s2 = env.sessions.session(s1->id());
TEST_EQUAL(s1, s2, "They should be the same session.");

env.sessions.takeSession(s1->id());

Session *s3 = env.sessions.newSession(PID, TMPL);
TEST_NOTEQUAL_STR(s1->id(), s3->id(), "Testing if IDs are unique.");

TEST_EQUAL_INT(env.sessions.size(), 1, "Testing if size is 1.");

std::string id = s3->id();
env.sessions.deleteSession(id);
Session *s4 = env.sessions.session(id);
TEST_EQUAL(s4, nullptr, "No session should be returned.");

TEST_EQUAL_INT(env.sessions.size(), 0, "Testing if size is 0.");

Session *s5 = env.sessions.newSession(PID, TMPL);
TEST_NOTEQUAL(s5, nullptr, "A new session was created.");

{
  Transaction transaction;
  transaction.cpr = PID;
  transaction.user = "me";

  Commit commit;
  commit.fields["field1"] = "hello";
  commit.fields["field2"] = "world";
  commit.templ = TMPL;
  
  Macro macro;
  macro.attributes["version"] = "1.0";
  macro.attributes["name"] = "somemacro";
  
  s5->commitMacro(transaction, commit, macro);
}
id = s5->id();

env.sessions.store();

// Resume session using session id.
Session *s6 = env.sessions.session(id);
TEST_NOTEQUAL(s6, nullptr, "We did get one right?");
TEST_EQUAL_STR(s6->id(), id, "Did we get the stored session?");

{
  Transaction transaction;
  transaction.cpr = PID;
  transaction.user = "me";

  Commit commit;
  commit.fields["field1"] = "hello";
  commit.fields["field2"] = "world";
  commit.templ = TMPL;
  
  Macro macro;
  macro.attributes["version"] = "1.0";
  macro.attributes["name"] = "somemacro";
  
  s6->commitMacro(transaction, commit, macro);
}
s6->nocommit(); 

env.sessions.store();

// Resume session using patientid/template tupple.
Session *s7 = env.sessions.newSession(PID, TMPL);
TEST_NOTEQUAL(s7, nullptr, "We did get one right?");
TEST_EQUAL_STR(s7->id(), id, "Did we get the stored session?");

// Get an existing session and test locking.
Session *s8 = env.sessions.session(s7->id());
s7->lock();
TEST_FALSE(s8->mutex.trylock(), "Session should be locked.")
s7->unlock();

{
  Transaction transaction;
  transaction.cpr = PID;
  transaction.user = "me";

  Commit commit;
  commit.fields["field1"] = "hello";
  commit.fields["field2"] = "world";
  commit.templ = TMPL;
  
  Macro macro;
  macro.attributes["version"] = "1.0";
  macro.attributes["name"] = "somemacro";
  
  s7->commitMacro(transaction, commit, macro);
}
TEST_FALSE(s7->idle(), "Session is active.");
TEST_FALSE(s7->isReadonly(), "Not read only session.");
s7->setIdle(true); // Force idle
TEST_TRUE(s7->idle(), "Session is idle.");

Session *s9 = env.sessions.newSession(PID, TMPL"empty");
TEST_TRUE(s9->isReadonly(), "Read only session.");
TEST_FALSE(s9->idle(), "Readonly session is not idle.");
s9->setIdle(true); // Force idle (no effect)
TEST_FALSE(s9->idle(), "Readonly session still not idle.");
*/
TEST_END;

#endif/*TEST_SESSION*/

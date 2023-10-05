/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            journal.cc
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
#include "journal.h"

#include <hugin.hpp>

Journal::Journal() {}

void Journal::addEntry(Transaction &transaction, Commit &commit,
                       std::string resume, Template *templ,
                       LUAOnCommit *oncommit)
{
  size_t index = 0;
  std::vector< Macro >::iterator i = templ->macros.begin();
  while(i != templ->macros.end()) {
    Macro &m = *i;
    if(commit.macro == m.name) break;
    index++;
    i++;
  }

  if(index >= templ->macros.size()) {
    ERR(journal, "Could not find macro %s in template %s\n",
        commit.macro.c_str(), templ->name.c_str());
    //      return;
  } else {
    DEBUG(journal, "Found macro %s as index %u in template %s\n",
          commit.macro.c_str(), (int)index,
          templ->name.c_str());
  }

  if(entrylist.size() == 0) {
    //if(user() == "") setUser(transaction.user);
    if(patientID() == "") setPatientID(transaction.patientid);
  }

  DEBUG(journal, "addEntry: template(%s)\n",
        templ->name.c_str());

  // Test if the username or the cpr has changed... 
  // if so, commit and clear the list.
#if 0 // no - it breaks things...
  if(user() != transaction.user || patientID() != transaction.patientid) {
    this->commit();
    entrylist.clear();
  }
#endif

  addEntry(resume, commit.macro, transaction.user, index, oncommit);
}

void Journal::addEntry(std::string resume, std::string macro,
                       std::string user, int index, LUAOnCommit *oncommit)
{
  DEBUG(journal, "Add: %p %s %s - %s\n", this, macro.c_str(),
        user.c_str(), resume.c_str());

  ResumeEntry re;
  re.oncommit = oncommit;
  re.resume = resume;
  re.macro = macro;
  re.user = user;
  re.dirty = false;
  entrylist[index] = re;
}

std::string Journal::getEntry(std::string macro)
{
  DEBUG(journal, "Get: %p %s\n", this, macro.c_str());

  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.macro == macro) return i->second.resume;
    i++;
  }
  return "";
}

void Journal::removeEntry(std::string macro)
{
  DEBUG(journal, "Remove: %p %s\n", this, macro.c_str());

  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.macro == macro) {
      entrylist.erase(i);
      break;
    }
    i++;
  }
}

void Journal::setDirty(std::string macro)
{
  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.macro == macro) {
      i->second.dirty = true;
      break;
    }
    i++;
  }
}

bool Journal::dirty(std::string macro)
{
  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.macro == macro) {
      return i->second.dirty;
      break;
    }
    i++;
  }
  return false;
}
  
void Journal::setPatientID(std::string id)
{
  _patientid = id;
}

std::string Journal::patientID()
{
  return _patientid;
}

void Journal::runOnCommitScripts()
{
  std::map< int, ResumeEntry >::iterator i = entrylist.begin();
  while(i != entrylist.end()) {
    if(i->second.oncommit != nullptr) {
      try {
        i->second.oncommit->run();
      } catch(LUAScript::Exception &e) {
        throw e;
      }
    }
    i++;
  }
}


#ifdef TEST_JOURNAL
//deps: debug.cc log.cc journal_uploadserver.cc journal_commit.cc mutex.cc luascript.cc saxparser.cc luautil.cc configuration.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(LUA_LIBS) $(CURL_LIBS) $(EXPAT_LIBS)
#include <test.h>

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_JOURNAL*/

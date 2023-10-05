/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sessionserialiser.cc
 *
 *  Thu May 20 11:26:18 CEST 2010
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
#include "sessionserialiser.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "journal.h"

#include "sessionparser.h"
#include "sessionheaderparser.h"
#include "database.h"

#include "xml_encode_decode.h"
//#include "base64.h"

#include "environment.h"

#include <stdio.h>
#include <string.h>

#define PRE "pracro_session"

std::string getSessionFilename(const std::string &path,
                               const std::string &sessionid)
{
  return path + "/" PRE "." + sessionid;
}


static std::string itostr(int i)
{
  char sid[32];
  snprintf(sid, sizeof(sid), "%d", i);
  return sid;
}

SessionSerialiser::SessionSerialiser(Environment *e, std::string path)
 : env(e)
{
  this->path = path;
}

#define XENC(s) xml_encode(s)
#define XDEC(s) xml_decode(s)

//#define BENC(s) base64encode(s)
//#define BDEC(s) base64decode(s)

Session *SessionSerialiser::loadStr(const std::string &xml)
{
  //  SessionAutolock lock(*session);
  SessionParser parser;
  parser.parse(xml.data(), xml.length());

  Session *session = new Session(env,
                                 XDEC(parser.sessionid),
                                 XDEC(parser.patientid),
                                 XDEC(parser.templ));
  session->isreadonly = parser.status == "readonly";
  Journal *j = session->journal();
  //  j->setUser(XDEC(parser.userid));
  j->setPatientID(XDEC(parser.patientid));
  std::vector<SessionParser::Entry>::iterator i = parser.entries.begin();
  while(i != parser.entries.end()) {
    j->addEntry(XDEC(i->resume), XDEC(i->macro), XDEC(i->user), i->index,
                i->oncommit);
    i++;
  }

  //  session->database()->restore(XDEC(parser.database));

  return session;
}

std::string SessionSerialiser::saveStr(Session *session)
{
  //  SessionAutolock lock(*session);

  std::string xml;

  xml += "<?xml version='1.0' encoding='UTF-8'?>\n";
  xml += "<session timestamp=\"" + itostr(time(nullptr)) + "\""
    " status=\"" + XENC(session->isreadonly?"readonly":"") + "\""
    " id=\"" + XENC(session->id()) + "\""
    " template=\"" + XENC(session->templ) + "\""
    " patientid=\"" + XENC(session->patientid) + "\">\n";

  Journal *journal = session->journal();

  xml += "  <journal patientid=\"" + XENC(journal->patientID()) + "\">\n";

  std::map< int, Journal::ResumeEntry >::iterator i =
    journal->entrylist.begin();
  while(i != journal->entrylist.end()) {

    xml += "    <entry index=\""+itostr(i->first) + "\""
      " macro=\"" + XENC(i->second.macro) + "\""
      " user=\"" + XENC(i->second.user) + "\">\n";
    xml += "      <resume>" + XENC(i->second.resume) + "</resume>\n";
    LUAOnCommit *oncommit = i->second.oncommit;
    if(oncommit != nullptr) {
      xml += "      <oncommit>\n";

      xml += "        <envs>\n";
      std::map<LUAScript::env_t, std::string>::iterator ei =
        oncommit->_env.begin();
      while(ei != oncommit->_env.end()) {
        std::string id;
        switch(ei->first) {
        case LUAScript::ENV_PATIENTID: id = "ENV_PATIENTID"; break;
        case LUAScript::ENV_TEMPLATE: id = "ENV_TEMPLATE"; break;
        case LUAScript::ENV_MACRO: id = "ENV_MACRO"; break;
        case LUAScript::ENV_USER: id = "ENV_USER"; break;
        }
        
        xml += "          <env id=\"" + XENC(id) + "\">"+
          XENC(ei->second) + "</env>\n";
        ei++;
      }
      xml += "        </envs>\n";

      xml += "        <values>\n";
      std::map<std::string, std::string>::iterator vi =
        oncommit->values.begin();
      while(vi != oncommit->values.end()) {
        xml += "          <value name=\"" + XENC(vi->first) + "\">"+
          XENC(vi->second) + "</value>\n";
        vi++;
      }
      xml += "        </values>\n";

      xml += "        <scripts>\n";
      std::vector<std::pair<std::string, std::string> >::iterator si =
        oncommit->scripts.begin();
      while(si != oncommit->scripts.end()) {
        xml += "          <script name=\"" + XENC(si->second) + "\">"+
          XENC(si->first) + "</script>\n";
        si++;
      }
      xml += "        </scripts>\n";
      
      xml += "      </oncommit>\n";
    }
    xml += "    </entry>\n";

    i++;
  }

  xml += "  </journal>\n";

  std::string dbtype = "pgsql";
  xml += "  <database type=\""+dbtype+"\">"+
    //    XENC(session->database()->serialise())+
    "</database>\n";

  xml += "</session>\n";

  return xml;
}

Session *SessionSerialiser::load(const std::string &sessionid)
{
  // read xml from file
  std::string filename = getSessionFilename(path, sessionid);

  FILE *fp =  fopen(filename.c_str(), "r");
  std::string xml;
  while(!feof(fp)) {
    char str[64];
    memset(str, 0, sizeof(str));
    fread(str, sizeof(str) - 1, 1, fp);
    xml += str;
  }
  fclose(fp);

  Session *session = loadStr(xml);

  // delete file
  unlink(filename.c_str());
  
  return session;
}

void SessionSerialiser::save(Session *session)
{
  std::string filename = getSessionFilename(path, session->id());

  std::string xml = saveStr(session);

  // write xml to file
  FILE *fp =  fopen(filename.c_str(), "w");
  if(!fp) {
    ERR(sessionserialiser, "Could not write session to file %s\n",
        filename.c_str());
    return;
  }
  fwrite(xml.data(), xml.size(), 1, fp);
  fclose(fp);
}

static inline bool isxmlfile(std::string name)
{
  if(name.find(PRE,0) == std::string::npos) return false;

  struct stat s;
  stat(name.c_str(), &s);

  if(S_ISREG(s.st_mode) == false) return false;
  
  return true;
}

Session *SessionSerialiser::findFromTupple(const std::string &patientid,
                                           const std::string &templ)
{
  DEBUG(sessionserialiser, "Looking for: PatientID %s - Template %s\n",
        patientid.c_str(), templ.c_str());

  DIR *dir = opendir(path.c_str());
  if(!dir) {
    ERR(sessionserialiser, "Could not open directory: %s - %s\n",
        path.c_str(), strerror(errno));
    return nullptr;
  }

  struct dirent *dirent;
  while( (dirent = readdir(dir)) != nullptr ) {

    std::string filename = path+"/"+dirent->d_name;
    
    DEBUG(sessionserialiser, "Looking at file: %s\n", filename.c_str());

    if(isxmlfile(filename)) {

      DEBUG(sessionserialiser, "Is xml file\n");

      SessionHeaderParser::Header header;

      SessionHeaderParser p(filename);
      try {
        p.parse();
        header = p.getHeader();
      } catch( ... ) {
        continue;
      }

      if(header.patientid == patientid && header.templ == templ) {
        // Load session file
        FILE *fp =  fopen(filename.c_str(), "r");
        std::string xml;
        while(!feof(fp)) {
          char str[64];
          memset(str, 0, sizeof(str));
          fread(str, sizeof(str) - 1, 1, fp);
          xml += str;
        }
        fclose(fp);

        Session *session = loadStr(xml);

        DEBUG(sessionserialiser, "PatientID %s - Template %s\n", 
              session->patientid.c_str(),
              session->templ.c_str());

        closedir(dir);
        unlink(filename.c_str());
        return session;
      }
    }
  }

  closedir(dir);

  return nullptr;
}

std::map<std::string, SessionHeaderParser::Header>
SessionSerialiser::sessionFiles()
{
  std::map<std::string, SessionHeaderParser::Header> list;

  DIR *dir = opendir(path.c_str());
  if(!dir) {
    ERR(sessionserialiser, "Could not open directory: %s - %s\n",
        path.c_str(), strerror(errno));
    return list;
  }

  struct dirent *dirent;
  while( (dirent = readdir(dir)) != nullptr ) {

    std::string filename = path+"/"+dirent->d_name;
    
    if(isxmlfile(filename)) {

      SessionHeaderParser::Header header;

      SessionHeaderParser p(filename);
      try {
        p.parse();
        header = p.getHeader();
      } catch( ... ) {
        continue;
      }
      
      list[filename] = header;
    }
  }

  closedir(dir);

  return list;
  
}


#ifdef TEST_SESSIONSERIALISER
//deps: session.cc journal.cc debug.cc configuration.cc mutex.cc journal_commit.cc sessionparser.cc saxparser.cc xml_encode_decode.cc database.cc pracrodaopgsql.cc pracrodaotest.cc pracrodao.cc journal_uploadserver.cc log.cc environment.cc semaphore.cc artefact.cc macrolist.cc templatelist.cc entitylist.cc inotify.cc versionstr.cc exception.cc macroheaderparser.cc templateheaderparser.cc sessionheaderparser.cc luascript.cc luautil.cc courselist.cc courseparser.cc
//cflags: -I.. $(PTHREAD_CFLAGS) $(EXPAT_CFLAGS) $(PQXX_CFLAGS) -DWITHOUT_ARTEFACT $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(PTHREAD_LIBS) $(EXPAT_LIBS) $(PQXX_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include "test.h"

#include <stdio.h>

#define SID "42"
#define PID "1234567890"
#define TID "sometemplate"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

#if 0
// Make sure we start out on an empty session directory.
std::string spath = "/tmp/test_sessionserialiser";
while(mkdir(spath.c_str(), 0777) == -1 && errno == EEXIST) {
  spath += "X";
}

std::string xml;

debug_parse("+all");

Environment env;

{
  FILE *fp = fopen("/tmp/"PRE".42", "w");
  fclose(fp);
  TEST_TRUE(isxmlfile("/tmp/"PRE".42"), "Xml file");
  unlink("/tmp/"PRE".42");

  fp = fopen("/tmp/pracro_diller.42", "w");
  fclose(fp);
  TEST_FALSE(isxmlfile("/tmp/pracro_diller.42"), "Txt file");
  unlink("/tmp/pracro_diller.42");

  TEST_FALSE(isxmlfile("/tmp/"PRE".42"), "No file");
  TEST_FALSE(isxmlfile("/tmp/badname"), "No file, bad name");
}

{
  Session session(&env, SID, PID, TID);
  Journal *j = session.journal();
  j->addEntry("some text", "macro1", 0);
  j->addEntry("some more text", "macro2", 2);
  j->addEntry("yet some more text", "macro3", 1);
  SessionSerialiser s(&env, spath);
  xml = s.saveStr(&session);
  s.loadStr(xml);
  std::string xml2 = s.saveStr(&session);
  TEST_EQUAL_STR(xml, xml2, "Compare");
}

{
  Session session(&env, SID, PID, TID);
  Journal *j = session.journal();
  j->addEntry("some text", "macro1", 0);
  j->addEntry("some more text", "macro2", 2);
  j->addEntry("yet some more text", "macro3", 1);
  SessionSerialiser s(&env, spath);
  xml = s.saveStr(&session);
}
/*
{
  Session session(SID, PID, TID);
  SessionSerialiser s(spath);
  s.loadStr(xml);
  std::string xml2 = s.saveStr(&session);
  TEST_EQUAL_STR(xml, xml2, "Compare");
}
*/
{
  Session session(&env, SID, PID, TID);
  Journal *j = session.journal();
  j->addEntry("some text", "macro1", 0);
  j->addEntry("some more text", "macro2", 2);
  j->addEntry("yet some more text", "macro3", 1);
  SessionSerialiser s(&env, spath);
  s.save(&session);
}
/*
{
  Session session(SID, PID, TID);
  SessionSerialiser s(spath);
  s.load(SID);
  std::string xml2 = s.saveStr(&session);
  TEST_EQUAL_STR(xml, xml2, "Compare");
}
*/

{
  Session session(&env, SID, PID, TID);
  SessionSerialiser s(&env, spath);
  s.save(&session);
  Session *s1 = s.findFromTupple(PID, TID);
  TEST_NOTEQUAL(s1, nullptr, "Found it?");
  TEST_EQUAL(s1->id(), SID, "Compare found tuple.");
  TEST_EQUAL(s1->patientid, PID, "Compare found tuple.");
  TEST_EQUAL(s1->templ, TID, "Compare found tuple.");
  delete s1;
}
#endif/*0*/

TEST_END;

#endif/*TEST_SESSIONSERIALISER*/

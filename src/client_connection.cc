/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            client_connection.cc
 *
 *  Thu Feb  3 09:33:48 CET 2011
 *  Copyright 2011 Bent Bisballe Nyeng
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
#include "client_connection.h"

#include "transactionhandler.h"
#include "xml_encode_decode.h"

static std::string error_box(std::string message)
{
  std::string errorbox =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<pracro version=\"1.0\">\n"
    "  <error>" + message + "</error>\n"
    "</pracro>\n";
  return errorbox;
}

#ifdef TEST_CLIENT_CONNECTION
static bool did_commit = false;
#endif

ClientConnection::ClientConnection(Environment &e, headers_t headers,
                                   headers_t args, std::string uri)
  : env(e), parser(&transaction)
{
  DEBUG(connection, "[%p] CREATE\n", this);

  size_t i = 0;
  std::string *tar[3];
  tar[0] = &request.course;
  tar[1] = &request.templ;
  tar[2] = &request.macro;
  int p = -1;
  while(i < uri.size() && p < 3) {
    if(uri[i] == '/') {
      p++;
    } else if(p > -1) {
      *(tar[p]) += uri[i];
    } else {
      ERR(connection, "Malformed URI. Missing beginning '/'!");
    }
    i++;
  }

  DEBUG(connection, "Course: %s, Template: %s, Macro: %s\n",
        request.course.c_str(),
        request.templ.c_str(),
        request.macro.c_str());

  templ = request.templ;

  request.patientid = patientid = args.lookup("patientid");
  sessionid = args.lookup("sessionid");
  
  docommit = args.lookup("statechange") == "commit";
  donocommit = args.lookup("statechange") == "nocommit";
  dodiscard = args.lookup("statechange") == "discard";

#ifdef TEST_CLIENT_CONNECTION
  did_commit = false;
#endif

  parser_complete = true;
}

ClientConnection::~ClientConnection()
{
  DEBUG(connection, "[%p] DESTROY\n", this);
}

void ClientConnection::nocommit(Session *session)
{
  if(donocommit) {
    if(session->isReadonly()) { // NoCommit of an empty session discards it.
      dodiscard = true;
      return;
    }

    DEBUG(connection, "NoCommit (%s)\n", session->id().c_str());
    donocommit = false;
    session->nocommit();
  }
}

void ClientConnection::commit(Session *session)
{
  if(docommit) {
    if(session->isReadonly()) { // Commit of an empty session discards it.
      dodiscard = true;
      return;
    }

    DEBUG(connection, "Commit (%s)\n", session->id().c_str());
    std::string sid = session->id();
    try {
      session->commit();
    } catch(LUAScript::Exception &e) {
      throw e;
    } catch(Journal::Exception &e) {
      throw e;
    }
    env.sessions.deleteSession(sid);
    sessionid = "";
    docommit = false;
#ifdef TEST_CLIENT_CONNECTION
    did_commit = true;
#endif
  }
}

void ClientConnection::discard(Session *session)
{
  if(dodiscard) {
    DEBUG(connection, "Discard (%s)\n", session->id().c_str());
    std::string sid = session->id();
    session->discard();
    env.sessions.deleteSession(sid);
    sessionid = "";
    dodiscard = false;
  }
}

bool ClientConnection::data(const char *data, size_t size)
{
  DEBUG(connection, "data(%p %d)\n", data, (int)size);

  parser_complete = parser.parse(data, size);

  return true;
}

bool ClientConnection::handle()
{
  DEBUG(connection, "handle\n");

  if(patientid == "") {
    response = error_box(xml_encode("Missing patientid."));
    parser_complete = true;
    return true;
  }

  if(request.course == "") {
    response = error_box(xml_encode("Missing course."));
    parser_complete = true;
    return true;
  }

  Session *session = nullptr;
  SessionAutounlock l(&session);

  try {
    if(sessionid == "") {
      // Create new session
      // NOTE: New session is returned in locked state!
      session = env.sessions.newLockedSession(patientid, templ);
    } else {
      // Attach to old session
      // NOTE: Returned session is returned in locked state!
      session = env.sessions.lockedSession(sessionid);
      
      // Session didn't exist - create a new one anyway.
      if(session == nullptr) {
        // NOTE: New session is returned in locked state!
        session = env.sessions.newLockedSession(patientid, templ);
      }
    }
  } catch(Sessions::SessionAlreadyActive &e) {
    ERR(connection, "Session already active.\n");
    parser_complete = true;
    response = error_box(xml_encode("Session "+e.sessionid+" already active."));
    parser_complete = true;
    return true;
  }

  if(session == nullptr) {
    ERR(connection, "New session could not be created.\n");
    response = error_box(xml_encode("New session could not be created."));
    parser_complete = true;
    return true;
  }
  
  sessionid = session->id();

  // Force session discard on empty template name.
  if(templ == "") dodiscard = true;

  try {
    if(parser_complete) {
      response = handleTransaction(request, transaction, env, *session);
    }

    try {
      commit(session);
    } catch(LUAScript::Exception &e) {
      response = error_box(xml_encode(e.msg));
      return true;
    } catch(Journal::Exception &e) {
      response = error_box(xml_encode(e.msg));
      return true;
    }
    nocommit(session);
    discard(session);
    
    return true;
    
  } catch(...) {
    ERR(connection, "Failed to parse data!\n");
    response = error_box(xml_encode("XML Parse error."));
    return true;
  }

  return false;
}

void ClientConnection::getReply(Httpd::Reply &reply)
{
  headers_t hdrs;

  hdrs["Content-Type"] = "text/plain; charset=UTF-8";
  hdrs["SessionID"] = sessionid;

  reply.headers = hdrs;

  if(parser_complete == false)
    reply.data = error_box(xml_encode("XML Parser need more data."));
  else
   reply.data = response;

  reply.status = 200; // http 'OK'
}

#ifdef TEST_CLIENT_CONNECTION
//deps: debug.cc transactionparser.cc session.cc xml_encode_decode.cc saxparser.cc transactionhandler.cc journal.cc mutex.cc templateparser.cc exception.cc configuration.cc macroparser.cc semaphore.cc entitylist.cc luaquerymapper.cc inotify.cc log.cc queryhandlerpentominos.cc widgetgenerator.cc queryhandlerpracro.cc journal_commit.cc versionstr.cc luaresume.cc luautil.cc artefact.cc environment.cc database.cc macrolist.cc templatelist.cc pracrodao.cc templateheaderparser.cc macroheaderparser.cc pracrodaotest.cc pracrodaopgsql.cc journal_uploadserver.cc sessionserialiser.cc sessionparser.cc widgetvalue.cc courseparser.cc courselist.cc luascript.cc sessionheaderparser.cc luaoncommit.cc
//cflags: -DWITHOUT_DATABASE -DWITHOUT_ARTEFACT -I.. $(LUA_CFLAGS) $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS) $(PQXX_CXXFLAGS) $(CURL_CFLAGS)
//libs: $(LUA_LIBS) $(EXPAT_LIBS) $(PTHREAD_LIBS) $(PQXX_LIBS) $(CURL_LIBS)
#include "test.h"
/*
static char xml_request[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <request macro=\"test\" template=\"test\"/>\n"
"</pracro>\n"
  ;

static char xml_commit[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <commit version=\"\" macro=\"referral\" template=\"amd_forunders\" >\n"
"  <field value=\"Some docs\" name=\"referral.doctor\"/>\n"
"  <field value=\"DIMS\" name=\"referral.diagnosecode\"/>\n"
"  <field value=\"Avs\" name=\"referral.diagnose\"/>\n"
" </commit>\n"
"</pracro>\n"
  ;

static char xml_commit_p1[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <commit version=\"\" macro=\"referral\" template=\"amd_forunders\" >\n"
"  <field value=\"Some docs\" name=\"referral.doctor\"/>\n"
"  <field value=\"DIMS\" name=\"referral.diagn"
  ;

static char xml_commit_p2[] =
"ose\"/>\n"
" </commit>\n"
"</pracro>\n"
  ;
*/

static const char empty_reply[] = 
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<pracro version=\"1.0\">\n"
  "</pracro>\n";

#include "configuration.h"

TEST_BEGIN;

debug_parse("-all,+connection,+session");

Conf::xml_basedir = "../xml";

Environment env;
Httpd::Reply reply;
std::string sid;

// Without data
{
  headers_t hdrs;
  ClientConnection con(env, hdrs, hdrs, "/test");
  TEST_TRUE(con.handle(), "Test handler return value.");
  con.getReply(reply);
  TEST_NOTEQUAL_STR(reply.data, empty_reply, "Did we get nonemtpy reply?");
  TEST_NOTEQUAL_STR(reply.headers["SessionID"], "",
                    "Did we get a new session id?");
  sid = reply.headers["SessionID"];
  TEST_FALSE(did_commit, "No commit.");
}

{
  headers_t hdrs;
  ClientConnection con(env, hdrs, hdrs, "/test/test"); 
  TEST_TRUE(con.handle(), "Test handler return value.");
  con.getReply(reply);
  TEST_EQUAL_STR(reply.data, empty_reply, "Did we get an empty reponse?");
  TEST_NOTEQUAL_STR(reply.headers["SessionID"], "", "Non empty session id?");
  TEST_NOTEQUAL_STR(reply.headers["SessionID"], sid, "Not the same id!");
  TEST_FALSE(did_commit, "No commit.");
}
/*
{
  headers_t hdrs;
  ClientConnection con(env, hdrs, hdrs, "");
  TEST_TRUE(con.handle("", 0), "Test handler return value.");
  con.getReply(reply);
  TEST_EQUAL_STR(reply.data, "", "Test response value.");
  TEST_EQUAL_STR(reply.headers["SessionID"], "", "Test existing session id.");
  TEST_TRUE(did_commit, "Commit.");
}

{
  headers_t hdrs;
  ClientConnection con(env, hdrs, hdrs, "");
  TEST_TRUE(con.handle("", 0), "Test handler return value.");
  con.getReply(reply);
  TEST_EQUAL_STR(reply.data, "", "Test response value.");
  TEST_NOTEQUAL_STR(reply.headers["SessionID"], "", "Test existing session id.");
  TEST_NOTEQUAL_STR(reply.headers["SessionID"], sid, "Test new session id.");
  TEST_FALSE(did_commit, "No commit.");
}
*/
/*
// With commit partial data
{
  ClientConnection con(env, "", false);
  TEST_FALSE(con.handle(xml_commit_p1, sizeof(xml_commit_p1) - 1),
             "Test handler return value.");
  sid = con.getSessionID();
  TEST_NOTEQUAL_STR(sid, "", "Test new session id.");
  TEST_FALSE(did_commit, "No commit.");
  TEST_EQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_TRUE(con.handle(xml_commit_p2, sizeof(xml_commit_p2) - 1),
            "Test handler return value.");
  TEST_EQUAL_STR(con.getSessionID(), sid, "Test session id.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_FALSE(did_commit, "No commit.");
}

// With commit partial data and journal commit
{
  ClientConnection con(env, "", true);
  TEST_FALSE(con.handle(xml_commit_p1, sizeof(xml_commit_p1) - 1),
             "Test handler return value.");
  sid = con.getSessionID();
  TEST_NOTEQUAL_STR(sid, "", "Test new session id.");
  TEST_EQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_FALSE(did_commit, "No commit.");
  TEST_TRUE(con.handle(xml_commit_p2, sizeof(xml_commit_p2) - 1),
            "Test handler return value.");
  TEST_EQUAL_STR(con.getSessionID(), "", "Test session id.");
  TEST_TRUE(did_commit, "No commit.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
}

// With commit data
{
  ClientConnection con(env, "", false);
  TEST_TRUE(con.handle(xml_commit, sizeof(xml_commit) - 1),
             "Test handler return value.");
  sid = con.getSessionID();
  TEST_NOTEQUAL_STR(sid, "", "Test new session id.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_FALSE(did_commit, "No commit.");
}

{
  ClientConnection con(env, sid, false); 
  TEST_TRUE(con.handle(xml_commit, sizeof(xml_commit) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_NOTEQUAL_STR(con.getSessionID(), "", "Test existing session id.");
  TEST_EQUAL_STR(con.getSessionID(), sid, "Test existing session id.");
  TEST_FALSE(did_commit, "No commit.");
}

{
  ClientConnection con(env, sid, true);
  TEST_TRUE(con.handle(xml_commit, sizeof(xml_commit) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_EQUAL_STR(con.getSessionID(), "", "Test existing session id.");
  TEST_TRUE(did_commit, "Commit.");
}

{
  ClientConnection con(env, sid, false);
  TEST_TRUE(con.handle(xml_commit, sizeof(xml_commit) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_NOTEQUAL_STR(con.getSessionID(), "", "Test existing session id.");
  TEST_NOTEQUAL_STR(con.getSessionID(), sid, "Test new session id.");
  TEST_FALSE(did_commit, "No commit.");
}

// With request data
{
  ClientConnection con(env, "", false);
  TEST_TRUE(con.handle(xml_request, sizeof(xml_request) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  sid = con.getSessionID();
  TEST_NOTEQUAL_STR(sid, "", "Test new session id.");
  TEST_FALSE(did_commit, "No commit.");
}

{
  ClientConnection con(env, sid, false); 
  TEST_TRUE(con.handle(xml_request, sizeof(xml_request) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_NOTEQUAL_STR(con.getSessionID(), "", "Test existing session id.");
  TEST_EQUAL_STR(con.getSessionID(), sid, "Test existing session id.");
  TEST_FALSE(did_commit, "No commit.");
}

{
  ClientConnection con(env, sid, true);
  TEST_TRUE(con.handle(xml_request, sizeof(xml_request) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getResponse(), "", "Test response value.");
  TEST_EQUAL_STR(con.getSessionID(), "", "Test existing session id.");
  TEST_TRUE(did_commit, "Commit.");
}

{
  ClientConnection con(env, sid, false);
  TEST_TRUE(con.handle(xml_request, sizeof(xml_request) - 1),
             "Test handler return value.");
  TEST_NOTEQUAL_STR(con.getSessionID(), "", "Test existing session id.");
  TEST_NOTEQUAL_STR(con.getSessionID(), sid, "Test new session id.");
  TEST_FALSE(did_commit, "No commit.");
}
*/
TEST_END;

#endif/*TEST_CLIENT_CONNECTION*/

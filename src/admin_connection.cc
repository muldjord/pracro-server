/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            admin_connection.cc
 *
 *  Thu Feb  3 09:33:44 CET 2011
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
#include "admin_connection.h"

#include <stdlib.h>

#include "admin_rc.h"
#include "admin_export.h"

#include <hugin.hpp>

#include "configuration.h"
#include "sessionserialiser.h"

static std::string admin_sessionunlock(Environment &env, std::string id)
{
  // NOTE: Returned session is returned in locked state!
  Session *session = nullptr;
  SessionAutounlock l(&session);

  session = env.sessions.lockedSession(id);
  if(session) {
    if(session->isReadonly()) {
      env.sessions.deleteSession(id);
      return "Session " + id + " was 'readonly' and has been discarded.";
    } else {
      session->setIdle(true);
      return "Session " + id + " has been deactivated (set to idle).";
    }
  }
  return "Session " + id + " does not exist or has been committed.";
}

static std::string admin_listactivesessions(Environment &env)
{
  std::string str;

  std::vector<Sessions::SessionInfo> act = env.sessions.activeSessions();
  std::vector<Sessions::SessionInfo>::iterator i = act.begin();
  while(i != act.end()) {
    str += "Session " + i->id + ": "+i->templ+" on "+i->patientid+" "+
        std::string(i->idle?"[idle]":"[active]")+"\n";
    i++;
  }

  SessionSerialiser ser(&env, Conf::session_path);
  std::map<std::string, SessionHeaderParser::Header> files = ser.sessionFiles();

  std::map<std::string, SessionHeaderParser::Header>::iterator j = files.begin();
  while(j != files.end()) {
    std::string file = j->first;
    SessionHeaderParser::Header header = j->second;

    str += "Session " + header.id + ": " + header.templ + " on "
      + header.patientid + " [session file: " + file + "]\n";

    j++;
  }

  return str;
}

static std::string admin_header(std::string uri)
{
  return admin_rc("header1") + uri + admin_rc("header2");
}

static std::string admin_flush(Environment &env)
{
  env.sessions.store();
  return "All sessions flushed to disc in " + Conf::session_path + ".";
}

AdminConnection::AdminConnection(Environment &e, headers_t a, std::string u)
  : env(e), args(a), uri(u) {}

AdminConnection::~AdminConnection() {}

bool AdminConnection::data(const char *, size_t) { return true; }

bool AdminConnection::handle()
{
  status = 200; // OK

  DEBUG(admin, "URI: %s\n", uri.c_str());

  if(uri == "/") {
    reply = admin_header(uri) +
      "<h2>Pracro server v" VERSION "</h2>"
      "<h3>Command list:</h3>"
      "<p><strong>/sessionunlock?id=<em>[ID]</em></strong> unlock session with [ID] as its session id.\n"
      "<strong>/listactivesessions</strong> lists all active sessions on the server.\n"
      "<strong>/flushsessions</strong> flushes all active sessions to disc.\n"
      "<strong>/export?template=<em>[TEMPLATE]</em></strong> export template with name [TEMPLATE] to a csv file (comma-separated-values file, that can be opened and viewed in most spreadsheet software).\n"
      "<strong>/export?template=<em>[TEMPLATE]</em>&from=<em>[FROM]</em>&to=<em>[TO]</em></strong> export template with name [TEMPLATE] and time range to a csv file. Both <em>from</em> and <em>to</em> are in unixtime.\n\n"
      "All commands also require <strong>&password=<em>[PASSWORD]</em></strong> to be set.</p>\n"
      + admin_rc("footer");
    return true;
  }

  if(args.find("password") == args.end() || args["password"] != Conf::server_passwd) {
    reply = admin_header(uri) +
      "'" + uri + "' query is either missing password or password is incorrect."
      + admin_rc("footer");
    return true;
  }

  if(uri == "/sessionunlock" && args.find("id") != args.end()) {
    reply = admin_header(uri) + admin_sessionunlock(env, args["id"])
      + admin_rc("footer");
    return true;
  }
  
  if(uri == "/listactivesessions") {
    reply = admin_header(uri) + admin_listactivesessions(env)
      + admin_rc("footer");
    return true;
  }
  
  if(uri == "/flushsessions") {
    reply = admin_header(uri) + admin_flush(env) + admin_rc("footer");
    return true;
  }
  
  if(uri == "/export" && args.find("template") != args.end()) {
    time_t from = 0;
    if(args.find("from") != args.end()) from = atoi(args["from"].c_str());
    
    time_t to = time(nullptr);
    if(args.find("to") != args.end()) to = atoi(args["to"].c_str());
    bool ok;
    std::string res = admin_export(env, args["template"], &ok, from, to);
    if(!ok) reply = admin_header(uri) + res + admin_rc("footer");
    else {
      reply = res;
      hdrs["Content-Type"] = "text/csv; charset=UTF-8";
      hdrs["Content-Disposition"] = "attachment; filename=\""+args["template"]+".csv\"";
    }
    return true;
  }

  if(uri == "/favicon.ico") {
    hdrs["Content-Type"] = "image/ico";
    reply = admin_rc("favicon");
    return true;
  }
  
  reply = admin_header(uri) +
    "'" + uri + "' not recognised as a valid command."
    + admin_rc("footer");
  return true;
}

void AdminConnection::getReply(Httpd::Reply &r)
{
  if(hdrs.find("Content-Type") == hdrs.end())
    hdrs["Content-Type"] = "text/html; charset=UTF-8";

  r.data = reply;
  r.headers = hdrs;
  r.status = status; // http 'OK'
}

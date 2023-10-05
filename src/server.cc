/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            server.cc
 *
 *  Wed Aug 22 12:16:03 CEST 2007
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
#include "server.h"

#include <config.h>
#include <stdio.h>
#include <unistd.h>

#include "httpd.h"
#include "configuration.h"
#include "environment.h"

#include "connection.h"
#include "client_connection.h"
#include "admin_connection.h"

extern std::string logfile;
extern volatile bool logfile_reload;

class PracroHttpd : public Httpd {
public:
  PracroHttpd() {}
  ~PracroHttpd()
  {
    env.sessions.store();
  }

  void error(const std::string &err)
  {
    fprintf(stderr, "ERROR: %s\n", err.c_str());
    fflush(stderr);
  }

  void *begin(const std::string &url,
              const std::string &method,
              const std::string &version,
              headers_t &getargs,
              headers_t &headers)
  {
    Connection *connection = nullptr;

    if(headers.find("User-Agent") != headers.end() &&
       headers["User-Agent"].find("Pracro") == std::string::npos) { // Admin
      connection = new AdminConnection(env, getargs, url);
    } else { // Pracro client
      connection = new ClientConnection(env, headers, getargs, url);
    }

    return connection;
  }

  bool data(void *ptr, const char *data, unsigned int data_size)
  {
    if(ptr) {
      Connection *connection = (Connection *)ptr;
      connection->data(data, data_size);
    }
    return true;
  }

  bool complete(void *ptr, Httpd::Reply &reply)
  {
    if(ptr) {
      Connection *connection = (Connection *)ptr;

      // Flush and do commit/discards
      if(!connection->handle()) return false;

      connection->getReply(reply);
    }

    return true;
  }

  void cleanup(void *ptr)
  {
    if(ptr) {
      Connection *connection = (Connection *)ptr;
      delete connection;
    }
  }

private:
  Environment env;
};



extern bool pracro_is_running;
void server()
{
  PracroHttpd httpd;

#ifndef WITHOUT_SSL
  if(Conf::use_ssl) httpd.listen_ssl(Conf::server_port,
                                     Conf::ssl_key,
                                     Conf::ssl_cert,
                                     Conf::connection_limit,
                                     Conf::connection_timeout);
  else
#endif
    httpd.listen(Conf::server_port,
                 Conf::connection_limit,
                 Conf::connection_timeout);

  while(pracro_is_running) sleep(1);

  DEBUG(server, "Server gracefully shut down.\n");
}

#ifdef TEST_SERVER
//deps: httpd.cc session.cc configuration.cc journal.cc journal_uploadserver.cc journal_commit.cc mutex.cc debug.cc sessionserialiser.cc sessionparser.cc saxparser.cc environment.cc semaphore.cc artefact.cc xml_encode_decode.cc database.cc pracrodaopgsql.cc pracrodaotest.cc pracrodao.cc entitylist.cc macrolist.cc templatelist.cc macroheaderparser.cc templateheaderparser.cc versionstr.cc exception.cc log.cc inotify.cc client_connection.cc admin_connection.cc admin_rc.cc admin_export.cc transactionparser.cc transactionhandler.cc templateparser.cc macroparser.cc queryhandlerpracro.cc luaquerymapper.cc queryhandlerpentominos.cc luaresume.cc widgetgenerator.cc widgetvalue.cc luascript.cc luautil.cc sessionheaderparser.cc courseparser.cc courselist.cc luaoncommit.cc
//cflags: -I.. -DWITHOUT_ARTEFACT -DWITHOUT_DB $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS) $(HTTPD_CFLAGS) $(LUA_CFLAGS) $(CURL_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS) $(HTTPD_LIBS) $(LUA_LIBS) $(CURL_LIBS)
#include <test.h>

#include <sys/types.h>
#include <signal.h>

// Thsese are globals normally defined in pracrod.cc:
std::string logfile;
volatile bool logfile_reload;
bool pracro_is_running = true;

char request[] = 
  "<?xml version='1.0' encoding='UTF-8'?>\n"
  "<pracro cpr=\"2003791613\" version=\"1.0\">\n"
  "  <request macro=\"example\" template=\"example\"/>\n"
  "</pracro>\n";

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  Conf::xml_basedir = "../xml/";
  // Make sure wo don't interrupt an already running server.
  Conf::server_port = 32100;
  Conf::database_backend = "testdb";
  pid_t pid = fork();

  switch(pid) {
  case -1: // error
    perror("fork() failed!\n");
    return 1;
    
  case 0: // child
    try {
      server();
    } catch(Exception &e) {
      printf(e.what());
      return 1;
    }
    return 0;
    
  default: // parent
    try {
      //      sleep(1); // Make sure the server is started.
      TCPSocket socket;
      socket.connect("localhost", Conf::server_port);
      
      socket.write(request);

      //      sleep(1); // Make sure the server has handled the request.
      char buf[32];
      memset(buf, 0, sizeof(buf));
      //      while(socket.read(buf, 31, 1000)) {
      while(socket.read(buf, 31, 1000000)) {
        printf(buf); fflush(stdout);
        memset(buf, 0, sizeof(buf));
      }
    } catch(Exception &e) {
      printf(e.what());
      kill(pid, SIGKILL); // Kill the server again.
      return 1;
    }
    kill(pid, SIGKILL); // Kill the server again.
    return 0;
  }
*/
TEST_END;

#endif/*TEST_SERVER*/

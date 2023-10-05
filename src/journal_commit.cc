/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            journal_commit.cc
 *
 *  Tue Mar 18 11:10:00 CET 2008
 *  Copyright 2008 Bent Bisballe Nyeng and Peter Skaarup
 *  deva@aasimon.org and piparum@piparum.dk
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
#include "journal_commit.h"

#include <config.h>

#include <hugin.hpp>

#include <string>

// for gethostbyname
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <endian.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "template.h"
#include "templateparser.h"

static int mwrite(int sock, const char *fmt, ...)
{
  int l = 0;
  va_list args;
  char *buffer;

  buffer = (char*)calloc(64*1024, 1);

  va_start(args, fmt);
  l = vsnprintf(buffer, 64*1024, fmt, args);
  va_end(args);
  
  if(sock != -1 && write(sock, buffer, l) != l) {
    ERR(journal, "write did not write all the bytes in the buffer.\n");
  }
 
  DEBUG(journal, "%s", buffer);
  free(buffer);

  return l;
}

int journal_commit(const char *cpr, const char *user,
                   const char *addr, unsigned short int port,
                   const char *buf, size_t size)
{
  int sock = -1;

#ifndef WITHOUT_UPLOADSERVER
  struct sockaddr_in sin;

  // Do DNS lookup
  char *ip;
  struct in_addr **addr_list;
  struct hostent *he;
  he = gethostbyname(addr);
  if(!he || !he->h_length) {
    ERR(journal, "gethostbyname(%s) failed (errno=%d)!\n", addr, errno);
    return -1;
  }

  addr_list = (struct in_addr **)he->h_addr_list;
  //  Get first value. We know for sure that there are at least one.
  ip = inet_ntoa(*addr_list[0]);

  // open socket
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(ip);
  sin.sin_port = htons(port);
  
  if( (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    ERR(journal, "Socket() failed!\n");
    return -1;
  }
  
  if(connect(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    ERR(journal, "Connect() failed: %s!\n", strerror(errno));
    return -1;
  }
#else
  sock = open("/tmp/pracro_journal.log", O_CREAT | O_WRONLY | O_TRUNC, 0600);
#endif/*WITHOUT_UPLOADSERVER*/

  // send header
  mwrite(sock, "PUT JOURNAL PROTO1.0 \r\n");
  mwrite(sock, "size : %i\r\n", size);
  mwrite(sock, "user: %s\r\n", user);
  mwrite(sock, "generator: pracro\r\n");
  //  mwrite(sock, "password: 1234\r\n");
  mwrite(sock, "cpr: %s\r\n", cpr);
  mwrite(sock, "date: %i\r\n", time(nullptr));
  mwrite(sock, "charset: utf8\r\n");
  mwrite(sock, "\r\n");

  std::string resume = buf;//stripTrailingWhitepace(addNewlines(buf, 60));

  // send body
  if(sock != -1 && write(sock, resume.c_str(), resume.size()) != (ssize_t)resume.size()) {
    ERR(journal, "write did not write all the bytes in the buffer.\n");
    return -1;
  }
  DEBUG(journal, "%s\n", buf);

#ifndef WITHOUT_UPLOADSERVER
  // close socket (write to network)
  close(sock);
#else
  // close socket (write to file)
  close(sock);
#endif/*WITHOUT_UPLOADSERVER*/

  return 0;
}

#ifdef TEST_JOURNAL_COMMIT
//deps: debug.cc log.cc mutex.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include <test.h>

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_JOURNAL_COMMIT*/

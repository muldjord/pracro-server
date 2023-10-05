/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            client_connection.h
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
#ifndef __PRACRO_CLIENT_CONNECTION_H__
#define __PRACRO_CLIENT_CONNECTION_H__

#include <string>

#include "connection.h"

#include "environment.h"
#include "transaction.h"
#include "transactionparser.h"

#include "luascript.h"

class Session;

class ClientConnection : public Connection {
public:
  /**
   * URI: course/template/macro
   * Params: petientid, sessionid and statechange (commit, nocommit or discard)
   * Headers are currently not used.
   */
  ClientConnection(Environment &e, headers_t headers,
                   headers_t args, std::string uri);
  ~ClientConnection();

  bool data(const char *data, size_t size);
  bool handle();

  void getReply(Httpd::Reply &reply);

private:
  void commit(Session *session);
  void nocommit(Session *session);
  void discard(Session *session);

  Environment &env;

  Transaction transaction;
  TransactionParser parser;

  std::string response;

  bool parser_complete;

  std::string sessionid;
  std::string patientid;
  std::string templ;
  bool docommit;
  bool dodiscard;
  bool donocommit;

  Request request;
};

#endif/*__PRACRO_CLIENT_CONNECTION_H__*/

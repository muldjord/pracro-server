/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            artefact.cc
 *
 *  Tue Jan  5 14:45:34 CET 2010
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
#include "artefact.h"

#include <hugin.hpp>
#include "configuration.h"

#include "queryparser.h"

Artefact::Artefact()
{
#ifndef WITHOUT_ARTEFACT

  DEBUG(artefact, "Creating artefact connection %s : %d\n",
               Conf::artefact_addr.c_str(), Conf::artefact_port);

  atfh = atf_init();
  if(!atfh) ERR(artefact, "Out of memory!\n");

  conn = atf_connect(atfh,
                     Conf::artefact_addr.c_str(),
                     Conf::artefact_port,
                     Conf::artefact_use_ssl);

#endif/*WITHOUT_ARTEFACT*/
}

Artefact::~Artefact()
{
#ifndef WITHOUT_ARTEFACT

  atf_disconnect(conn);
  atf_close(atfh);

#endif/*WITHOUT_ARTEFACT*/
}

#ifndef WITHOUT_ARTEFACT
static QueryResult node2result(atf_result_node_t *node, time_t timestamp)
{
  QueryResult rnode;
  rnode.timestamp = timestamp;
  rnode.source = "artefact";

  if(!node) return rnode;

  struct _atf_result_node_t *child = node->child;
  while(child) {
    if(child->value == nullptr) {
      rnode.groups[child->name] = node2result(child, timestamp);
    } else {
      rnode.values[child->name] = child->value;
    }
    child = child->next;
  }

  return rnode;
}
#endif/*WITHOUT_ARTEFACT*/

QueryResult Artefact::exec(Query &query,
                           std::string patientid,
                           std::string user)
{
  QueryResult rroot;
  rroot.timestamp = 0;
  rroot.source = "pentominos";

#ifndef WITHOUT_ARTEFACT

  atf_transaction_t* atft = nullptr;
  atf_reply_t *reply = nullptr;
  atf_result_t *result = nullptr;
  atf_result_node_t *root = nullptr;
  atf_status_t status;
  time_t timestamp;
  atf_id id;

  if(query.attributes.find("class") == query.attributes.end()) {
    ERR(artefact, "Missing 'class' attribute!\n");
    goto aaarg;
  }

  atft = atf_new_transaction(conn, patientid.c_str());
  if(!atft) goto aaarg;

  id = atf_add_query(atft, query.attributes["class"].c_str(),
                     FILTER_LATEST, USE_NONE, 0, 0);
  if(!atft) goto aaarg;

  reply = atf_commit(atft);
  if(!reply) goto aaarg;

  if(atf_get_num_results(reply, id) != 1) goto aaarg;

  result = atf_get_result(reply, id, 0);
  if(!result) goto aaarg;

  status = atf_get_result_status(result, nullptr, 0);
  if(status != ATF_STATUS_OK) goto aaarg;
 
  timestamp = atf_get_result_timestamp(result);

  if(query.attributes.find("ttl") != query.attributes.end()) {
    std::string ttl = query.attributes["ttl"];
    time_t xml_ttl = time(nullptr) - atol(ttl.c_str());
    // Check if ttl from xml makes this result too old
    if(timestamp < xml_ttl) {
      // Set to pentominos_max_ttl - 1000 to make it too old thereby discarding result
      timestamp = (time(nullptr) - Conf::pentominos_max_ttl) - 1000;
    }
  }

  rroot.timestamp = timestamp;

  root = atf_get_result_node(result);
  if(!root) goto aaarg;

  {
    QueryResult qresult = node2result(root, timestamp);
    rroot.groups[query.attributes["class"]] = qresult;
  }

  goto cleanup;

 aaarg:
  ERR(artefact, "Artefact comm error (%d)!\n", atf_get_last_error(atfh));

 cleanup:
  if(root) atf_free_result_node(root);
  if(reply) atf_free_reply(reply);
  if(atft) atf_free_transaction(atft);

#endif/*WITHOUT_ARTEFACT*/

  return rroot;
}


#ifdef TEST_ARTEFACT
//deps: configuration.cc debug.cc log.cc mutex.cc
//cflags:  -I.. $(ATF_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(ATF_LIBS) $(PTHREAD_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
debug_parse("+all");

{
  Artefact atf;
  Query q;
  q.attributes["class"] = "echo";
  QueryResult res = atf.exec(q, "0000000000", "me");

  res.print();
}

{
  Conf::artefact_addr = "nowhere_at_all.com";
  Conf::artefact_port = 10000;

  Artefact atf;
  Query q;
  q.attributes["class"] = "echo";
  QueryResult res = atf.exec(q, "0000000000", "me");

  res.print();
}
*/
TEST_END;

#endif/*TEST_ARTEFACT*/

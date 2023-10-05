/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configuration.h
 *
 *  Tue Aug 15 23:57:12 CEST 2006
 *  Copyright  2006 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Artefact.
 *
 *  Artefact is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Artefact is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Artefact; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#ifndef __ARTEFACT_CONFIGURATION_H__
#define __ARTEFACT_CONFIGURATION_H__

#include <string>

#include <time.h>
#include <sys/types.h>
#include <stdint.h>

typedef uint16_t port_t;

namespace Conf {
  extern port_t server_port;
  extern std::string server_user;
  extern std::string server_group;
  extern std::string server_passwd;

  extern std::string journal_commit_addr;
  extern port_t journal_commit_port;

  extern time_t db_max_ttl;
  extern time_t pentominos_max_ttl;

  extern std::string artefact_addr;
  extern port_t artefact_port;
  extern bool artefact_use_ssl;

  extern int artefact_poolsize;

  extern std::string database_backend;
  extern std::string database_addr;
  extern std::string database_user;
  extern std::string database_passwd;

  extern int database_poolsize;

  extern std::string xml_basedir;

  extern bool use_ssl;
  extern std::string ssl_key;
  extern std::string ssl_cert;

  extern int connection_limit;
  extern int connection_timeout;

  extern std::string session_path;
  extern std::string session_discard_path;
};

#endif/*__ARTEFACT_CONFIGURATION_H__*/

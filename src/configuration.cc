/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configuration.cc
 *
 *  Tue Aug 15 23:57:13 CEST 2006
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
#include "configuration.h"

#include <config.h>

port_t Conf::server_port = 12345;
std::string Conf::server_user = "pracro";
std::string Conf::server_group = "pracro";
std::string Conf::server_passwd = "pracro";

std::string Conf::journal_commit_addr = "localhost";
port_t Conf::journal_commit_port = 18112;

time_t Conf::db_max_ttl = 7 * 60 * 60 * 24;
time_t Conf::pentominos_max_ttl = 7 * 60 * 60 * 24;

std::string Conf::artefact_addr = "localhost";
port_t Conf::artefact_port = 11108;
bool Conf::artefact_use_ssl = false;

int Conf::artefact_poolsize = 1;

std::string Conf::database_backend = "pgsql";
std::string Conf::database_addr = "localhost";
std::string Conf::database_user = "pracro";
std::string Conf::database_passwd = "pracro";

int Conf::database_poolsize = 2;

std::string Conf::xml_basedir = XML;

bool Conf::use_ssl = false;
std::string Conf::ssl_key = "";
std::string Conf::ssl_cert = "";

int Conf::connection_limit = 42;
int Conf::connection_timeout = 0;

std::string Conf::session_path = "/tmp";
std::string Conf::session_discard_path = "";

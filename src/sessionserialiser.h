/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sessionserialiser.h
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
#ifndef __PRACRO_SESSIONSERIALISER_H__
#define __PRACRO_SESSIONSERIALISER_H__

#include <string>
#include <map>

#include "sessionheaderparser.h"

#include "session.h"

class Environment;

class SessionSerialiser {
public:
  SessionSerialiser(Environment *e, std::string path);

  Session *loadStr(const std::string &xml);
  std::string saveStr(Session *session);

  Session *findFromTupple(const std::string &patientid,
                          const std::string &templ);

  Session *load(const std::string &sessionid);
  void save(Session *session);

  std::map<std::string, SessionHeaderParser::Header> sessionFiles();

private:
  std::string path;
  Environment *env;
};

std::string getSessionFilename(const std::string &path,
                               const std::string &sessionid);

#endif/*__PRACRO_SESSIONSERIALISER_H__*/

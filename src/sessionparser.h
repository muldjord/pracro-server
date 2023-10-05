/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sessionparser.h
 *
 *  Thu May 20 14:30:23 CEST 2010
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
#ifndef __PRACRO_SESSIONPARSER_H__
#define __PRACRO_SESSIONPARSER_H__

#include "saxparser.h"

#include "luaoncommit.h"

#include <string>
#include <vector>

class SessionParser : public SAXParser {
public:
  SessionParser();
  ~SessionParser();

  void characterData(std::string &data);
  void startTag(std::string name, attributes_t &attr);
  void endTag(std::string name);
  void parseError(const char *buf, size_t len, std::string error, int lineno);
  
  std::string status;
  std::string templ;
  std::string sessionid;
  std::string patientid;
  std::string userid;
  std::string database;
  std::string dbtype;

  class Entry {
  public:
    int index;
    LUAOnCommit *oncommit;
    std::string macro;
    std::string resume;
    std::string user;
  };

  std::vector<Entry> entries;

private:
  bool inresume;
  bool inscript;
  bool invalue;
  bool inenv;
  std::string valuename;
  bool indatabase;
  LUAScript::env_t envid;
};

#endif/*__PRACRO_SESSIONPARSER_H__*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            transaction.h
 *
 *  Fri Aug 31 09:52:27 CEST 2007
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
#ifndef __PRACRO_TRANSACTION_H__
#define __PRACRO_TRANSACTION_H__

#include <string>
#include <vector>
#include <map>

class Request {
public:
  std::string macro;
  std::string templ;
  std::string course;
  
  std::string patientid;
};
typedef std::vector< Request > Requests;

typedef std::map< std::string, std::string > Fields;

class Commit {
public:
  std::string templ;
  std::string macro;
  std::string version;
  Fields fields;
};
typedef std::vector< Commit > Commits;

class Transaction {
public:
  std::string user;
  std::string patientid;
  std::string version;

  Commits commits;
};

#endif/*__PRACRO_TRANSACTION_H__*/

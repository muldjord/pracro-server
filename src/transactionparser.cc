/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            transactionparser.cc
 *
 *  Fri Aug 31 09:30:06 CEST 2007
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
#include "transactionparser.h"

#include <stdio.h>
#include <string.h>
#include <expat.h>

#include <string>
#include <map>

#include <hugin.hpp>
#include "exception.h"

TransactionParser::TransactionParser(Transaction *transaction)
{
  this->transaction = transaction;
  done = false;
  totalbytes = 0;
}

void TransactionParser::startTag(std::string name, attributes_t &attr)
{
  DEBUG(transactionparser, "<%s>\n", name.c_str());

  if(name == "pracro") {
    transaction->user = attr["user"];
    transaction->patientid = attr["patientid"];
    transaction->version = attr["version"];
  }

  if(name == "commit") {
    Commit c;
    c.templ = attr["template"];
    c.macro = attr["macro"];
    c.version = attr["version"];
    transaction->commits.push_back(c);
  }

  if(name == "field") {
    if(!transaction->commits.size()) {
      ERR(transactionparser, "Field without a commit tag!");
      throw std::exception();
    }

    if(attr.find("name") == attr.end()) {
      ERR(transactionparser, "Field is missing 'name' attribute");
      throw std::exception();
    }

    if(attr.find("value") == attr.end()) {
      ERR(transactionparser, "Field is missing 'value' attribute");
      throw std::exception();
    }

    transaction->commits.back().fields[attr["name"]] = attr["value"];
  }
}

void TransactionParser::parseError(const char *buf, size_t len,
                                   std::string error, int lineno)
{
  ERR(transactionparser, "TransactionParser error at line %d: %s\n",
      lineno, error.c_str());

  std::string xml;
  xml.append(buf, len);

  ERR(transactionparser, "\tBuffer %u bytes: [%s]\n", (int)len, xml.c_str());

  throw std::exception();
}

#ifdef TEST_TRANSACTIONPARSER
//deps: saxparser.cc debug.cc exception.cc log.cc mutex.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include "test.h"

static char xml_minimal[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro/>\n"
  ;

static char xml_request[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <request macro=\"test\" template=\"test\"/>\n"
"</pracro>\n"
  ;

static char xml_commit[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <commit version=\"\" macro=\"referral\" template=\"amd_forunders\" >\n"
"  <field value=\"Some docs\" name=\"referral.doctor\"/>\n"
"  <field value=\"DIMS\" name=\"referral.diagnosecode\"/>\n"
"  <field value=\"Avs\" name=\"referral.diagnose\"/>\n"
" </commit>\n"
"</pracro>\n"
  ;

static char xml_fail[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <request macro=\"test template=\"test\"/>\n"
"</pracro>\n"
  ;

TEST_BEGIN;

// Test minimal
{
  Transaction transaction;
  TransactionParser parser(&transaction);
  TEST_NOEXCEPTION(parser.parse(xml_minimal, sizeof(xml_minimal)-1), "minimal");
}

// Test request
{
  Transaction transaction;
  TransactionParser parser(&transaction);
  TEST_NOEXCEPTION(parser.parse(xml_request, sizeof(xml_request)-1), "request");
}

// Test commit
{
  Transaction transaction;
  TransactionParser parser(&transaction);
  TEST_NOEXCEPTION(parser.parse(xml_commit, sizeof(xml_commit)-1), "commit");
}

// Test parse error (should throw an exception)
{
  Transaction transaction;
  TransactionParser parser(&transaction);
  TEST_EXCEPTION(parser.parse(xml_fail, sizeof(xml_fail)-1), std::exception, "parse error");
}

TEST_END;

#endif/*TEST_TRANSACTIONPARSER*/

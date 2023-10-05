/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            transactionparser.h
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
#ifndef __PRACRO_TRANSACTIONPARSER_H__
#define __PRACRO_TRANSACTIONPARSER_H__

#include "saxparser.h"
#include "tcpsocket.h"
#include "transaction.h"

/**
 * Transaction parser class.
 * It parses all transactions amde to the server, and generates a Transaction
 * structure from it.
 * To activate the parser, the parser(char *buf, size_t size) buffer parser
 * method in the SAXParser parent class is used.
 * @see class SAXParser
 */
class TransactionParser : public SAXParser {
public:
  /**
   * Constructor.
   * It does nothing but set the internal transaction variable really.
   * @param transaction The Transaction to be filled with the parsed data.
   */
  TransactionParser(Transaction *transaction);
  
  /**
   * Start tag callback method.
   */
  void startTag(std::string name, attributes_t &attr);

  /**
   * Parser error callback method. Unlike its parent class, this method throws
   * an exception.
   */
  void parseError(const char *buf, size_t len, std::string error, int lineno);

private:
  Transaction *transaction;
};

#endif/*__PRACRO_TRANSACTIONPARSER_H__*/

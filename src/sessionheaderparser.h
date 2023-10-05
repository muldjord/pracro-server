/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            sessionheaderparser.h
 *
 *  Thu Aug  9 09:06:32 CEST 2012
 *  Copyright 2012 Bent Bisballe Nyeng
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
#ifndef __PRACRO_SESSIONHEADERPARSER_H__
#define __PRACRO_SESSIONHEADERPARSER_H__

#include "saxparser.h"

/**
 * Partial session parser.
 * This class is used to parse only the first tag of a session xml file.
 * The parser will run about 10 times faster than the one parsing the entire
 * file (see the SessionParser class) and can be used to find a
 * patientid/template match.
 * This class inherits the SAXParser baseclass.
 * Use the parse() method to run the parser, and collect the result via the
 * getPatientID() and getTemplate() methods.
 * If the file does not contain a session, or the file is not a valid xml file,
 * an Exception is thrown.
 */
class SessionHeaderParser : public SAXParser {
public:
  class Header {
  public:
    std::string patientid;
    std::string templ;
    std::string id;
  };

  /**
   * Constructor.
   * @param sessionfile A std::string containing the name of the file to parse.
   */
  SessionHeaderParser(std::string sessionfile);

  /**
   * Destructor.
   */
  ~SessionHeaderParser();

  /**
   * Overloaded parser callback method.
   */
  void startTag(std::string name, attributes_t &attr);

  /**
   * Overloaded parser callback method.
   */
  void parseError(const char *buf, size_t len, std::string error, int lineno);

  /**
   * Get a pointer to the parsed macro.
   * NOTE: The allocated memory for the macro is owned by the parser, and will be
   * freed upon parser deletion.
   * @return A pointer to the macro or nullptr on error.
   */
  Header getHeader();

protected:
  /**
   * Overloaded parser callback method.
   */
  int readData(char *data, size_t size);

private:
  int fd;

  bool done;
 
  Header header;

  std::string file;
  // Error callback function.
  void error(const char* fmt, ...);
};

#endif/*__PRACRO_SESSIONHEADERPARSER_H__*/

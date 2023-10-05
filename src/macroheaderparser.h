/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macroheaderparser.h
 *
 *  Wed Jul 22 08:42:23 CEST 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#ifndef __PRACRO_MACROHEADERPARSER_H__
#define __PRACRO_MACROHEADERPARSER_H__

#include "saxparser.h"
#include "template.h"

/**
 * Partial macro parser.
 * This class is used to parse only the first tag of a macro xml file.
 * The parser will run about 10 times faster than the one parsing the entire file
 * (see the MacroParser class) and can be used to make a list of which macros are in
 * which files.
 * This class inherits the SAXParser baseclass.
 * Use the parse() method to run the parser, and collect the result via the
 * getMacro() method.
 * If the file does not contain a macro, or the files is not a valid xml file, an 
 * Exception will be thrown.
 */
class MacroHeaderParser : public SAXParser {
public:
  /**
   * Constructor.
   * @param macrofile A std::string containing the name of the file to parse.
   */
  MacroHeaderParser(std::string macrofile);

  /**
   * Destructor.
   * Frees the allocated Macro* (if any).
   */
  ~MacroHeaderParser();

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
  Macro *getMacro();

protected:
  /**
   * Overloaded parser callback method.
   */
  int readData(char *data, size_t size);

private:
  int fd;
  Macro *m;

  std::string file;
  // Error callback function.
  void error(const char* fmt, ...);
};

#endif/*__PRACRO_MACROHEADERPARSER_H__*/

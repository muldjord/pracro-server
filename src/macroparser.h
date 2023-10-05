/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            macroparser.h
 *
 *  Wed Jun  4 11:57:38 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
#ifndef __PRACRO_MACROPARSER_H__
#define __PRACRO_MACROPARSER_H__

#include "saxparser.h"
#include "template.h"

class MacroParser : public SAXParser {
  typedef enum {
    UNDEFINED,
    MACRO,
    RESUME,
    RESUME_SCRIPT,
    QUERIES,
    QUERY,
    MAPS,
    MAP,
    WIDGETS,
    SCRIPTS,
    SCRIPT,
    COMMIT_SCRIPTS,
    COMMIT_SCRIPT
  } ParserState;

public:
  MacroParser(std::string macrofile);
  ~MacroParser();

  void characterData(std::string &data);
  void startTag(std::string name, attributes_t &attr);
  void endTag(std::string name);
  void parseError(const char *buf, size_t len, std::string error, int lineno);

  /**
   * Get a pointer to the parsed macro.
   * NOTE: The allocated memory for the macro is owned by the parser, and will be
   * freed upon parser deletion.
   * @return A pointer to the macro or nullptr on error.
   */
  Macro *getMacro();

protected:
  int readData(char *data, size_t size);

private:
  int fd;

  std::string file;

  // Parser state data
  ParserState state;
  Macro *m;
  Map *current_map;
  Script *current_script;
  Script *current_resume_script;
  Script *current_commit_script;
  std::vector< Widget* > widgetstack;

  // Error callback function.
  void error(const char* fmt, ...);
};

#endif/*__PRACRO_MACROPARSER_H__*/

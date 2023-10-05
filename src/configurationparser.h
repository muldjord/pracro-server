/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configurationparser.h
 *
 *  Wed Jul 30 11:48:31 CEST 2008
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
#ifndef __PRACRO_CONFIGURATIONPARSER_H__
#define __PRACRO_CONFIGURATIONPARSER_H__

#include <libconfig.h++>
#include <string>

#include <exception>

/**
 * This is the pentominos configuration class.\n
 * It simply wraps the libconfig c++ interface. It can be found at 
 * http://www.hyperrealm.com/libconfig/libconfig.html\n
 * To find out how the interface works, see
 * http://www.hyperrealm.com/libconfig/libconfig_manual.html#The-C_002b_002b-API
 */
class ConfigurationParser : public libconfig::Config {
public:
  /**
   * This exception is thrown by Configuration when reload fails.
   */
  class ParseException: public std::exception {
  public:
    ParseException(int line, std::string err) : l(line), e(err) {}
    ~ParseException() {}
    const char *what();

  private:
    std::string _what;
    int l;
    std::string e;
  };

  /**
   * This exception is thrown by Configuration when file read fails.
   */
  class ReadException: public std::exception {};
  

  /**
   * Constructor.\n
   * @param filename The filename to be loaded.
   */
  ConfigurationParser(std::string filename);

  /**
   * reload, simply reloads the configuration file attached to the configuration
   * object.
   */
  void reload();

private:
  std::string filename;
};

#endif/*__PRACRO_CONFIGURATIONPARSER_H__*/

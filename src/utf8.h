/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            utf8.h
 *
 *  Tue Feb 27 19:18:23 CET 2007
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
#ifndef __ARTEFACT_UTF8_H__
#define __ARTEFACT_UTF8_H__

#include <string>
#include <map>

#include "exception.h"

/**
 * This exception is thrown by UTF8 when the subsystem fails to initialize.
 */
class UTF8CreateException: public Exception {
public:
  UTF8CreateException(std::string reason) : 
    Exception("Error during creation of the UTF8 subsystem: " + reason) {}
};

/**
 * This exception is thrown by UTF8 when the subsystem fails encode the gives string.
 */
class UTF8EncodeException: public Exception {
public:
  UTF8EncodeException(std::string reason) : 
    Exception("Error during UTF8 encoding: " + reason) {}
};

/**
 * This exception is thrown by UTF8 when the subsystem fails decode the gives string.
 */
class UTF8DecodeException: public Exception {
public:
  UTF8DecodeException(std::string reason) : 
    Exception("Error during UTF8 decoding: " + reason) {}
};

/**
 * UTF-8 handler class.\n
 * It is used to convert between UTF-8 and some native charset Default
 * is ISO-8859-1. (Currently only the ISO-8859-1 charset is implemented!)
 */
class UTF8 {
public:
  /**
   * Constructor.
   * @param encoding A string containing native charset. Default is ISO-8859-1
   */
  UTF8(std::string encoding = "ISO-8859-1");
  
  /**
   * Encode a string from native encoding to UTF-8
   * @param s The string to encode.
   * @return The UTF-8 encoded string.
   */
  std::string encode(std::string s);
  
  /**
   * Decode a string from UTF-8 to native encoding.
   * @param s The UTF-8 string to decode.
   * @return The decoded string.
   */
  std::string decode(std::string s);
  
private:
  std::string encoding;
  
  std::map< std::string, std::string > map_encode;
  std::map< std::string, std::string > map_decode;
};

#endif/*__ARTEFACT_UTF8_H__*/

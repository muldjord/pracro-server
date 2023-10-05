/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            saxparser.h
 *
 *  Mon Mar 24 14:40:15 CET 2008
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
#ifndef __PRACRO_SAXPARSER_H__
#define __PRACRO_SAXPARSER_H__

#include <string>
#include <map>
#include <expat.h>

typedef std::map< std::string, std::string> attributes_t;

/**
 * This class implements a SAX Parser, utilising the eXpat XML parser library.
 * It uses virtual methods for the callbacks, and transforms tagnames and
 * attributes into C++ values (std::string and std::vector).
 */
class SAXParser {
public:
  /**
   * Constructor.
   * It initialises the eXpat library.
   */
  SAXParser();

  /**
   * Destructor.
   * It frees the eXpat library resources.
   */
  virtual ~SAXParser();

  /**
   * Call this method to use the reimplemented readData method for input.
   * The entire document is parsed through this single call.
   * @return An integer wityh value 0 on success, or 1 on failure.
   * @see int readData(char *data, size_t size)
   */
  int parse();

  /**
   * Character data callback method.
   * Reimplement this to get character callbacks.
   * This callback might be called several times, if a character block is big.
   * In that cae it might be nessecary to buffer to received bytes.
   * @param data A std::string containing the character data.
   */
  virtual void characterData(std::string &data);

  /**
   * Start tag callback mehtod.
   * Reimplement this to get start tag callbacks.
   * It is called each time a new start tag is seen.
   * @param name A std::string containing the tag name.
   * @param attributes A std::map of std::string to std::string containing all
   * attributes for the tag.
   */
  virtual void startTag(std::string name, attributes_t &attr);

  /**
   * End tag callback mehtod.
   * Reimplement this to get end tag callbacks.
   * It is called each time an end tag is seen.
   * @param name A std::string containing the tag name.
   */
  virtual void endTag(std::string name);

  /**
   * Error callback method.
   * Reimplement this to handle error messages.
   * A default implementation prints out the current buffer, linenumber and
   * error message to the screen.
   * @param buf A char* containing the current buffer being parsed.
   * @param len A size_t containing the length of the current buffer being
   * parsed.
   * @param error A std::string containing the error message.
   * @param lineno An integer containing the line number on which the error
   * occurred.
   */
  virtual void parseError(const char *buf, size_t len, std::string error,
                          int lineno);

  /**
   * Buffer parse method.
   * Use this method to parse an external buffer with xml data.
   * This method can be called several times (ie. in a read loop).
   * @param buf A char* containing the buffer to parse.
   * @param size A size_t comntaining the size of the buffer to parse.
   * @return A boolean with the value true if a complete document has been seen.
   * false otherwise.
   * @see bool parse(char *buf, size_t size)
   */
  bool parse(const char *buf, size_t size);

  /**
   * Get the number of bytes used from the last buffer.
   * If the buffer parse method is used, and the buffer comes from a stream of
   * xml doxuments, this method can be used to figure out how many bytes from
   * the stream should be replayed, to another parser.
   * @return an integer containing the number of bytes used from the last
   * buffer.
   * @see bool parse(char *buf, size_t size)
   */
  unsigned int usedBytes();

  // private stuff that needs to be public!
  std::string outertag;
  bool done;

protected:
  /**
   * Read data callback method.
   * This method is used when the parse() method is used.
   * It can be used to connect the parser with eg. a file.
   * @param data A char* containing the buffer to be filled.
   * @param size A size_t containing the maximum number of bytes to be filled
   * (ie. the size of data)
   * @return An integer contaning the actual number of bytes filled. 0 if no
   * more bytes are available.
   * @see int parse()
   */ 
  virtual int readData(char *data, size_t size);

  XML_Parser p;

  unsigned int bufferbytes;
  unsigned int totalbytes;
};

#endif/*__PRACRO_SAXPARSER_H__*/

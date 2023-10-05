/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            exception.h
 *
 *  Mon Mar 19 11:22:22 CET 2007
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
#ifndef __ARTEFACT_EXCEPTION_H__
#define __ARTEFACT_EXCEPTION_H__

#include <exception>

#include <string>

/**
 * Exception is the base class for all Pentominos exceptions
 */
class Exception : public std::exception {
public:
  /**
   * The constuctor sets the error message (retained by the what() call) and
   * adds an entry to the syslog, using the Pentominos::log interface.
   */
  Exception(std::string what);

  /**
   * Destructor
   */
  virtual ~Exception() {}

  /**
   * what is used to gain textual information about the exception.
   * @return A const char pointer to a zero terminated string containing
   * textual information about the exception.
   */
  virtual const char* what();

private:
  std::string _what;
};


#endif/*__ARTEFACT_EXCEPTION_H__*/

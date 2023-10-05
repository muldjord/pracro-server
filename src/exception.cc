/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            exception.cc
 *
 *  Mon Mar 19 11:33:15 CET 2007
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
#include "exception.h"

#include <hugin.hpp>

#include "log.h"

Exception::Exception(std::string what)
{
  log(what);
  _what = what;
  
}

const char* Exception::what()
{
  return _what.c_str();
}

#ifdef TEST_EXCEPTION
//deps: log.cc
//cflags: -I..
//libs:

#undef TEST_EXCEPTION // 'fix' redefined test macro.
#include <test.h>

#define PRE "MyExtException has been thrown: "
#define MSG "Hello World!"

class MyException : public Exception {
public:
  MyException() :
    Exception("MyException has been thrown") {}
};

class MyExtException : public Exception {
public:
  MyExtException(std::string thingy) :
    Exception(PRE + thingy) {}
};

void throwit()
{
  throw MyException();
}

TEST_BEGIN;

TEST_EXCEPTION(throwit(), MyException, "Throw it.");

std::string w;
try {
  throw MyExtException(MSG);
} catch( MyExtException &e ) {
  w = e.what();
}

TEST_EQUAL_STR(w, PRE MSG, "We got a 'what'.");

TEST_END;

#endif/*TEST_EXCEPTION*/

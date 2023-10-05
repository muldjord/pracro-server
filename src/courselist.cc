/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            courselist.cc
 *
 *  Thu Jul  7 10:25:06 CEST 2011
 *  Copyright 2011 Bent Bisballe Nyeng
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
#include "courselist.h"

#include "courseparser.h"

#include <hugin.hpp>

CourseList::CourseList(std::string path)
  : EntityList(path, "course")
{
  rescan();
}

void CourseList::addFile(std::string file)
{
  if(file.substr(file.size() - 4) != ".xml") {
    DEBUG(courselist, "Skipping file: %s\n", file.c_str());
    return;
  }

  DEBUG(courselist, "Adding file: %s\n", file.c_str());
  CourseParser parser(file);
  try {
    parser.parse();
    Course *templ = parser.getCourse();
    insertEntity(templ->name,
                 templ->version,
                 file);
  } catch(Exception &e) {
    WARN(courselist, "Skipping %s: %s\n", file.c_str(), e.what());
  }
}

#ifdef TEST_COURSELIST
//deps: entitylist.cc mutex.cc inotify.cc debug.cc courseparser.cc saxparser.cc exception.cc versionstr.cc log.cc
//cflags: -I .. $(PTHREAD_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(PTHREAD_LIBS) $(EXPAT_LIBS)
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_COURSELIST*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macrolist.cc
 *
 *  Wed Jul 22 10:26:40 CEST 2009
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
#include "macrolist.h"

#include <utility>

#include "macroheaderparser.h"

#include <hugin.hpp>

MacroList::MacroList(std::string path)
  : EntityList(path, "macro")
{
  rescan();
}


void MacroList::addFile(std::string file)
{
  if(file.substr(file.size() - 4) != ".xml") {
    DEBUG(macrolist, "Skipping file: %s\n", file.c_str());
    return;
  }

  DEBUG(macrolist, "Adding file: %s\n", file.c_str());
  MacroHeaderParser parser(file);
  try {
    parser.parse();
    Macro *macro = parser.getMacro();
    insertEntity(macro->name, macro->version, file);
  } catch(Exception &e) {
    WARN(macrolist, "Skipping %s: %s\n", file.c_str(), e.what());
  }
}

#ifdef TEST_MACROLIST
//deps: entitylist.cc exception.cc saxparser.cc debug.cc log.cc inotify.cc versionstr.cc mutex.cc macroheaderparser.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#define MACRODIR "/home"  // We assume this directory exists and does not contain any xml files!

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  // Test sorting
  MacroList lst(MACRODIR);

  lst["macro1"][VersionStr("1.0")] = "macro1-1.0.xml";
  lst["macro1"][VersionStr("1.1")] = "macro1-1.1.xml";
  lst["macro1"][VersionStr("1.1.1")] = "macro1-1.1.1.xml";
  lst["macro1"][VersionStr("1.2")] = "macro1-1.2.xml";
  lst["macro2"][VersionStr("1.0")] = "macro2.xml";
  lst["macro3"][VersionStr("1.0")] = "macro3.xml";

  std::vector<std::string> refs;
  refs.push_back("macro1-1.2.xml");
  refs.push_back("macro1-1.1.1.xml");
  refs.push_back("macro1-1.1.xml");
  refs.push_back("macro1-1.0.xml");
  refs.push_back("macro2.xml");
  refs.push_back("macro3.xml");

  MacroList::iterator i = lst.begin();
  std::vector<std::string>::iterator k = refs.begin();
  while(i != lst.end()) {
    MacroListItem::iterator j = i->second.begin();
    while(j != i->second.end()) {
      printf("%s - v%s file: %s - should be %s\n",
             i->first.c_str(),
             ((std::string)j->first).c_str(),
             j->second.c_str(),
             k->c_str());
      if(j->second != *k) return 1;
      j++; k++;
    }
    i++;
  }

  // Test lookup of latest versions.
  std::string m1 = lst.getLatestVersion("macro1");
  printf("Latest macro1: %s\n", m1.c_str());
  if(m1 != MACRODIR"/macro1-1.2.xml") return 1;

  std::string m2 = lst.getLatestVersion("macro2");
  printf("Latest macro2: %s\n", m2.c_str());
  if(m2 != MACRODIR"/macro2.xml") return 1;

  std::string m3 = lst.getLatestVersion("macro3");
  printf("Latest macro3: %s\n", m3.c_str());
  if(m3 != MACRODIR"/macro3.xml") return 1;

  // Look for non existing macro (this should throw an exception)
  try {
    std::string m4 = lst.getLatestVersion("macro4");
  } catch(Exception &e) {
    printf("ERROR: %s\n", e.what());
    goto onandon;
  }
  return 1;
 onandon:
*/
TEST_END;

#endif/*TEST_MACROLIST*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            templatelist.cc
 *
 *  Thu Jul 30 08:52:30 CEST 2009
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
#include "templatelist.h"

#include "templateheaderparser.h"

#include <hugin.hpp>

TemplateList::TemplateList(std::string path)
  : EntityList(path, "template")
{
  rescan();
}

void TemplateList::addFile(std::string file)
{
  if(file.substr(file.size() - 4) != ".xml") {
    DEBUG(templatelist, "Skipping file: %s\n", file.c_str());
    return;
  }

  DEBUG(templatelist, "Adding file: %s\n", file.c_str());
  TemplateHeaderParser parser(file);
  try {
    parser.parse();
    Template *templ = parser.getTemplate();
    insertEntity(templ->name,
                 templ->version,
                 file);
  } catch(Exception &e) {
    WARN(templatelist, "Skipping %s: %s\n", file.c_str(), e.what());
  }
}

#ifdef TEST_TEMPLATELIST
//deps: entitylist.cc versionstr.cc exception.cc inotify.cc mutex.cc debug.cc log.cc templateheaderparser.cc saxparser.cc
//cflags: -I.. $(EXPAT_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(EXPAT_LIBS) $(PTHREAD_LIBS)
#include <test.h>

#define TEMPLATEDIR "/home"  // We assume this directory exists and does not contain any xml files!

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  // Test sorting
  TemplateList lst(TEMPLATEDIR);

  lst["template1"][VersionStr("1.0")] = "template1-1.0.xml";
  lst["template1"][VersionStr("1.1")] = "template1-1.1.xml";
  lst["template1"][VersionStr("1.1.1")] = "template1-1.1.1.xml";
  lst["template1"][VersionStr("1.2")] = "template1-1.2.xml";
  lst["template2"][VersionStr("1.0")] = "template2.xml";
  lst["template3"][VersionStr("1.0")] = "template3.xml";

  std::vector<std::string> refs;
  refs.push_back("template1-1.2.xml");
  refs.push_back("template1-1.1.1.xml");
  refs.push_back("template1-1.1.xml");
  refs.push_back("template1-1.0.xml");
  refs.push_back("template2.xml");
  refs.push_back("template3.xml");

  TemplateList::iterator i = lst.begin();
  std::vector<std::string>::iterator k = refs.begin();
  while(i != lst.end()) {
    TemplateListItem::iterator j = i->second.begin();
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
  std::string m1 = lst.getLatestVersion("template1");
  printf("Latest template1: %s\n", m1.c_str());
  if(m1 != TEMPLATEDIR"/template1-1.2.xml") return 1;

  std::string m2 = lst.getLatestVersion("template2");
  printf("Latest template2: %s\n", m2.c_str());
  if(m2 != TEMPLATEDIR"/template2.xml") return 1;

  std::string m3 = lst.getLatestVersion("template3");
  printf("Latest template3: %s\n", m3.c_str());
  if(m3 != TEMPLATEDIR"/template3.xml") return 1;

  // Look for non existing template (this should throw an exception)
  try {
    std::string m4 = lst.getLatestVersion("template4");
  } catch(Exception &e) {
    printf("ERROR: %s\n", e.what());
    goto onandon;
  }
  return 1;
 onandon:
 */
TEST_END;

#endif/*TEST_TEMPLATELIST*/

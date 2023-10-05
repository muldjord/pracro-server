/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            versionstr.cc
 *
 *  Wed Jul 22 11:41:32 CEST 2009
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
#include "versionstr.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

// Workaround - major, minor and patch are defined as macros when using _GNU_SOURCES
#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif
#ifdef patch
#undef patch
#endif

VersionStr::VersionStr(std::string v)
{
  memset(version, 0, sizeof(version));
  set(v);
}

VersionStr::VersionStr(size_t major, size_t minor, size_t patch)
{
  version[0] = major;
  version[1] = minor;
  version[2] = patch;
}

void VersionStr::set(std::string v)
{
  std::string num;
  size_t idx = 0;
  for(size_t i = 0; i < v.length(); i++) {
    if(v[i] == '.') {
      if(idx > 2) throw Exception("Version string is too long\n");
      version[idx] = atoi(num.c_str());
      idx++;
      num = "";
    } else if(v[i] >= '0' && v[i] <= '9') {
      num.append(1, v[i]);
    } else {
      throw Exception(std::string("Version string contains illegal character: [")+v[i]+"]\n");
    }
  }
  if(idx > 2) throw Exception("Version string is too long\n");
  version[idx] = atoi(num.c_str());
}

VersionStr::operator std::string() const
{
  std::string v;
  char *buf;
  if(patch()) {
    asprintf(&buf, "%d.%d.%d", (int)major(), (int)minor(), (int)patch());
  } else {
    asprintf(&buf, "%d.%d", (int)major(), (int)minor());
  }
  v = buf;
  free(buf);
  return v;
}
  
void VersionStr::operator=(std::string v)
{
  set(v);
}

bool VersionStr::operator<(const VersionStr &other) const
{
  if(other.major() < major()) return true;
  if(other.major() > major()) return false;
  if(other.minor() < minor()) return true;
  if(other.minor() > minor()) return false;
  if(other.patch() < patch()) return true;
  if(other.patch() > patch()) return false;
  return false;
}

size_t VersionStr::major() const
{
  return version[0];
}

size_t VersionStr::minor() const
{
  return version[1];
}

size_t VersionStr::patch() const
{
  return version[2];
}

#ifdef TEST_VERSIONSTR
//deps: log.cc exception.cc
//cflags: -I..
//libs:
#include <test.h>

#include <set>
#include <vector>

TEST_BEGIN;

TEST_TRUE(false, "No tests yet!");
/*
  // Test normal constructor and string conversion
  VersionStr v1("1.2.3");
  printf("VersionStr: %s\n", ((std::string)v1).c_str());
  if((std::string)v1 != "1.2.3") return 1;

  // Test normal secondary constructor and numeric version number verification.
  VersionStr _v1(1, 2, 3);
  printf("VersionStr: %s\n", ((std::string)_v1).c_str());
  if((std::string)_v1 != "1.2.3") return 1;
  if(_v1.major() != 1) return 1;
  if(_v1.minor() != 2) return 1;
  if(_v1.patch() != 3) return 1;

  // Test smaller version number and string conversion
  VersionStr v2("1.2");
  printf("VersionStr: %s\n", ((std::string)v2).c_str());
  if((std::string)v2 != "1.2") return 1;
  
  // Test even smaller version number and string conversion
  VersionStr v3("1");
  printf("VersionStr: %s\n", ((std::string)v3).c_str());
  if((std::string)v3 != "1.0") return 1;

  // Test too long version number (should throw an exception)
  try {
    VersionStr v4("1.2.3.4"); // too long
    printf("VersionStr: %s\n", ((std::string)v4).c_str());
    if((std::string)v4 != "1.2.3") return 1;
  } catch(Exception &e) {
    goto nextone;
  }
  return 1;

 nextone:
  // Test illegal character in version number (should throw an exception)
  try { 
    VersionStr v5("1.2.a"); // illegal character
    printf("VersionStr: %s\n", ((std::string)v5).c_str());
    if((std::string)v5 != "1.2") return 1;
  } catch(Exception &e) {
    goto nextoneagain;
  }
  return 1;

 nextoneagain:
  // Test string assignment
  VersionStr v6("1.0");
  v6 = "1.1";
  if((std::string)v6 != "1.1") return 1;
    
  // Test sorting
  std::set<VersionStr> versions;
  versions.insert(VersionStr("1.0")); // These two should be the same
  versions.insert(VersionStr("1.0.0"));

  versions.insert(VersionStr("2.0")); // These should be sorted
  versions.insert(VersionStr("1.1"));
  versions.insert(VersionStr("0.1"));
  versions.insert(VersionStr("1.0.1"));
  versions.insert(VersionStr("1.0.3"));
  versions.insert(VersionStr("1.0.2"));

  std::vector<std::string> refs; // Sorting reference values.
  refs.push_back("2.0");
  refs.push_back("1.1");
  refs.push_back("1.0.3");
  refs.push_back("1.0.2");
  refs.push_back("1.0.1");
  refs.push_back("1.0");
  refs.push_back("0.1");

  std::set<VersionStr>::iterator i = versions.begin();
  std::vector<std::string>::iterator j = refs.begin();
  while(i != versions.end()) {
    printf("%s should be %s\n", ((std::string)*i).c_str(), j->c_str());
    if(((std::string)*i) != *j) return 1;
    i++; j++;
  }
*/
TEST_END;

#endif/*TEST_VERSIONSTR*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            macrotool_util.cc
 *
 *  Fri Jul 10 09:11:28 CEST 2009
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
#include "util.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#include <hugin.hpp>
#include "configuration.h"

static std::vector<std::string> listdir(std::string path)
{
  std::vector<std::string> files;

  DIR* dir = opendir(path.c_str());
  if(!dir) {
    ERR(dump, "Could not open directory: %s\n", path.c_str());
    return files;
  }

  struct dirent *d;
  while((d = readdir(dir)) != 0) {
    //if(d->d_type == DT_DIR) {
    std::string name = d->d_name;
    if(name.length() >= 4 && name.substr(name.length() - 4) == ".xml")
      files.push_back(name);
    //}
  }
  closedir(dir);

  return files;
}

std::vector<std::string> getMacros()
{
  std::vector<std::string> macros = listdir(Conf::xml_basedir + "/macros");
  return macros;
}

std::vector<std::string> getTemplates()
{
  std::vector<std::string> templates = listdir(Conf::xml_basedir + "/templates");
  return templates;
}

void printcolumn(std::string text, size_t width)
{
  printf("%s", text.c_str());
  for(size_t i = text.length(); i < width; i++) printf(" ");
  printf("\t");
}

#ifdef TEST_UTIL
//deps: configuration.cc debug.cc log.cc mutex.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include <test.h>

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_UTIL*/

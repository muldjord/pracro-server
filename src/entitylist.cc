/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            entitylist.cc
 *
 *  Thu Jan 14 14:17:34 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#include "entitylist.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>

#include <hugin.hpp>

static inline bool isdir(std::string name)
{
  struct stat s;
  stat(name.c_str(), &s);
  return S_ISDIR(s.st_mode);
}

static inline bool isfile(std::string name)
{
  struct stat s;
  stat(name.c_str(), &s);
  return S_ISREG(s.st_mode);
}

static std::vector<std::string> listdir(std::string path)
{
  std::vector<std::string> files;

  DIR* dir = opendir(path.c_str());
  if(!dir) {
    ERR(entitylist, "Could not open directory: %s\n", path.c_str());
    return files;
  }

  struct dirent *d;
  while((d = readdir(dir)) != 0) {
    std::string name = d->d_name;

    if(name == "." || name == "..") continue;

    if(isdir(path + "/" + name)) {
      std::vector<std::string> sub = listdir(path + "/" + name);
      files.insert(files.end(), sub.begin(), sub.end());
      continue;
    }

    if(isfile(path + "/" + name)) {
      if(name.length() >= 4 && name.substr(name.length() - 4) == ".xml")
        files.push_back(path + "/" + name);
    }
  }
  closedir(dir);

  return files;
}

EntityList::EntityList(std::string entitypath, std::string entityname)
{
  MutexAutolock lock(mutex);

  this->entityname = entityname;
  this->entitypath = entitypath;
}

EntityList::~EntityList()
{
}

void EntityList::rescan()
{
  clear();
  inotify.clear();

  inotify.addDirectory(entitypath, WATCH_DEEP_FOLLOW,
                       IN_CLOSE_WRITE |
                       IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF |
                       IN_DELETE | IN_DELETE_SELF |
                       IN_CREATE);

  std::vector<std::string> entitys = listdir(entitypath);
  std::vector<std::string>::iterator i = entitys.begin();
  while(i != entitys.end()) {
    addFile(*i);
    i++;
  }

  {
    iterator i = begin();
    while(i != end()) {
      EntityListItem::iterator j = i->second.begin();
      while(j != i->second.end()) {
        DEBUG(entitylist, "%s - v%s file: %s\n",
              i->first.c_str(),
              ((std::string)j->first).c_str(),
              j->second.c_str());
        j++;
      }
      i++;
    }
  }
}

bool EntityList::removeFile(std::string file)
{
  // Check if the file is already in the tree.
  iterator i = begin();
  while(i != end()) {
    EntityListItem::iterator j = i->second.begin();
    while(j != i->second.end()) {
      if(file == j->second) {
        DEBUG(entitylist, "Removing file: %s\n", file.c_str());
        i->second.erase(j);
        if(i->second.size() == 0) erase(i);
        return true;
      }
      j++;
    }
    i++;
  }

  return false;
}

void EntityList::updateFile(std::string file)
{
  removeFile(file);
  addFile(file);
}

void EntityList::updateList()
{
  while(inotify.hasEvents()) {
    INotify::Event event = inotify.getNextEvent();

    DEBUG(entitylist, "Handling event %s on %s, with param %s\n",
          event.maskstr().c_str(),
          event.name().c_str(),
          event.file().c_str());

    if(event.isDir()) {
      if(event.isCreateEvent()) {
        // A new directory was ceated. Scan it for files.
        std::vector<std::string> entitys = listdir(event.name()+"/"+event.file());
        std::vector<std::string>::iterator i = entitys.begin();
        while(i != entitys.end()) {
          updateFile(*i);
          i++;
        }
      }

      if(event.isMoveSelfEvent()) rescan();
      if(event.isDeleteSelfEvent()) rescan();
      if(event.isDeleteEvent()) rescan();
      if(event.isMovedFromEvent()) rescan();
      if(event.isMovedToEvent()) rescan();

    } else {
      if(event.isCloseWriteEvent()) updateFile(event.name()+"/"+event.file());
      if(event.isMovedFromEvent()) removeFile(event.name()+"/"+event.file());
      if(event.isMovedToEvent()) updateFile(event.name()+"/"+event.file());
      if(event.isDeleteEvent()) removeFile(event.name()+"/"+event.file());
    }
  }
}

std::string EntityList::getLatestVersion(std::string entity)
{
  MutexAutolock lock(mutex);

  updateList();

  if(find(entity) == end()) {
    throw Exception("Entity ("+entityname+") ["+entity+"] does not exist");
  }

  EntityListItem mli = (*this)[entity];
  if(mli.size() == 0) {
    throw Exception("Entity ("+entityname+") ["+entity+"] does not exist.");
  }

  DEBUG(entitylist, "Search for %s - found %s v%s\n",
        entity.c_str(),
        mli.begin()->second.c_str(),
        ((std::string)mli.begin()->first).c_str());

  return mli.begin()->second;
}

void EntityList::insertEntity(std::string entity, std::string version, std::string file)
{
  std::pair<VersionStr, std::string> p(VersionStr(version), file);
  (*this)[entity].insert(p);
}

#ifdef TEST_ENTITYLIST
//deps: inotify.cc debug.cc mutex.cc exception.cc versionstr.cc log.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include "test.h"

#include <string.h>

#define _DIR "/tmp/entitylist_test_dir"

class TestList : public EntityList {
public:
  TestList(std::string path) : EntityList(path, "test") { rescan(); }

private:
  void addFile(std::string file)
  {
    char version[32];
    FILE *fp = fopen(file.c_str(), "r");
    if(!fp) return;
    memset(version, 0, sizeof(version));
    fread(version, sizeof(version), 1, fp);
    fclose(fp);
    
    fprintf(stderr, "Inserting: %s\n", version);
    insertEntity("test", version, file);
  }
};

bool createfile(TestList &lst, std::string filename,
                std::string name, std::string version)
{
  FILE *fp = fopen(filename.c_str(), "w");
  if(!fp) return false;
  fprintf(fp, "%s", version.c_str());
  fclose(fp);
  return true;
}

TEST_BEGIN;

debug_parse("-all,+entitylist");

if(mkdir(_DIR, 0777) == -1) TEST_FATAL("Could not create test dir.");

TestList lst(_DIR);

TEST_EXCEPTION(lst.getLatestVersion("test"), Exception,
               "Test lookup of macro in empty tree.");

if(!createfile(lst, _DIR"/file1.xml", "test", "1.0"))
  TEST_FATAL("Unable to write to the file");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/file1.xml", "Test");

if(!createfile(lst, _DIR"/file2.xml", "test", "2.0"))
  TEST_FATAL("Unable to write to the file");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/file2.xml", "Test");

unlink(_DIR"/file2.xml");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/file1.xml", "Test");

if(mkdir(_DIR"/more", 0777) == -1) TEST_FATAL("Could not create test dir.");

if(!createfile(lst, _DIR"/more/file1.xml", "test", "3.0"))
  TEST_FATAL("Unable to write to the file");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/more/file1.xml", "Test");

if(!createfile(lst, _DIR"/more/file2.xml", "test", "4.0"))
  TEST_FATAL("Unable to write to the file");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/more/file2.xml", "Test");

unlink(_DIR"/more/file2.xml");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/more/file1.xml", "Test");

rename(_DIR"/more/file1.xml", _DIR"/more/file2.xml");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/more/file2.xml", "Test");

rename(_DIR"/more", _DIR"/more2");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/more2/file2.xml", "Test");

rename(_DIR"/more2/file2.xml", _DIR"/file3.xml");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/file3.xml", "Test");

unlink(_DIR"/file3.xml");

rmdir(_DIR"/more2");

TEST_EQUAL_STR(lst.getLatestVersion("test"), _DIR"/file1.xml", "Test");

unlink(_DIR"/file1.xml");

TEST_EXCEPTION(lst.getLatestVersion("test"), Exception, "Test lookup of missing macro.");

rmdir(_DIR);

TEST_EXCEPTION(lst.getLatestVersion("test"), Exception, "Test lookup in missing folder.");

TEST_END;

#endif/*TEST_ENTITYLIST*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            inotify.cc
 *
 *  Wed Jan  6 09:58:47 CET 2010
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
#include "inotify.h"

#include <hugin.hpp>

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define TEST(x, m) ((x & m) == m)

#define STEST(x, m) (TEST(x, m)?#m" ":"")

static std::string mask2asc(uint32_t mask)
{
  std::string str;

  str += STEST(mask, IN_ACCESS);
  str += STEST(mask, IN_ATTRIB);
  str += STEST(mask, IN_CLOSE_WRITE);
  str += STEST(mask, IN_CLOSE_NOWRITE);
  str += STEST(mask, IN_CREATE);
  str += STEST(mask, IN_DELETE);
  str += STEST(mask, IN_DELETE_SELF);
  str += STEST(mask, IN_MODIFY);
  str += STEST(mask, IN_MOVE_SELF);
  str += STEST(mask, IN_MOVED_FROM);
  str += STEST(mask, IN_MOVED_TO);
  str += STEST(mask, IN_OPEN);

  str += STEST(mask, IN_ALL_EVENTS);
  str += STEST(mask, IN_CLOSE);
  str += STEST(mask, IN_MOVE);

  str += STEST(mask, IN_DONT_FOLLOW);
  str += STEST(mask, IN_MASK_ADD);
  str += STEST(mask, IN_ONESHOT);
  str += STEST(mask, IN_ONLYDIR);

  str += STEST(mask, IN_IGNORED);
  str += STEST(mask, IN_ISDIR);
  str += STEST(mask, IN_Q_OVERFLOW);
  str += STEST(mask, IN_UNMOUNT);

  return str;
}

static inline bool isdir(const char *name)
{
  struct stat s;
  stat(name, &s);
  return S_ISDIR(s.st_mode);
}

INotify::Event::Event(struct inotify_event *event, std::string name)
{
  this->_name = name;
  if(event) {
    this->_mask = event->mask;
    this->_file = event->name;
  } else {
    this->_mask = 0;
    this->_file = "";
  }

  DEBUG(inotify, "Event [%s] %s (%s)\n",
        mask2asc(_mask).c_str(),
        _name.c_str(),
        _file.c_str());
}

bool INotify::Event::isAttributeChangeEvent()
{
  return TEST(_mask, IN_ATTRIB);
}

bool INotify::Event::isCloseEvent()
{
  return isCloseWriteEvent() || isCloseNoWriteEvent();
}

bool INotify::Event::isCloseWriteEvent()
{
  return TEST(_mask, IN_CLOSE_WRITE);
}

bool INotify::Event::isCloseNoWriteEvent()
{
  return TEST(_mask, IN_CLOSE_NOWRITE);
}

bool INotify::Event::isCreateEvent()
{
  return TEST(_mask, IN_CREATE);
}

bool INotify::Event::isOpenEvent()
{
  return TEST(_mask, IN_OPEN);
}

bool INotify::Event::isModifyEvent()
{
  return TEST(_mask, IN_MODIFY);
}

bool INotify::Event::isAccessEvent()
{
  return TEST(_mask, IN_ACCESS);
}

bool INotify::Event::isDeleteEvent()
{
  return TEST(_mask, IN_DELETE);
}

bool INotify::Event::isDeleteSelfEvent()
{
  return TEST(_mask, IN_DELETE_SELF);
}

bool INotify::Event::isIgnoredEvent()
{
  return TEST(_mask, IN_IGNORED);
}

bool INotify::Event::isMoveSelfEvent()
{
  return TEST(_mask, IN_MOVE_SELF);
}

bool INotify::Event::isMovedFromEvent()
{
  return TEST(_mask, IN_MOVED_FROM);
}

bool INotify::Event::isMovedToEvent()
{
  return TEST(_mask, IN_MOVED_TO);
}

bool INotify::Event::isDir()
{
  return TEST(_mask, IN_ISDIR);
}

std::string INotify::Event::name()
{
  return _name;
}

std::string INotify::Event::file()
{
  return _file;
}

uint32_t INotify::Event::mask()
{
  return _mask;
}

std::string INotify::Event::maskstr()
{
  return mask2asc(_mask);
}

INotify::INotify()
{
  ifd = inotify_init1(O_NONBLOCK); 
  if(ifd == -1) {
    perror("Inotify init failed.\n");
    return;
  }
}

INotify::~INotify()
{
  if(ifd != -1) close(ifd);
}

void INotify::addFile(std::string name, uint32_t mask)
{
  // Extra mask bitflags:
  //IN_DONT_FOLLOW (since Linux 2.6.15)
  // Don't dereference pathname if it is a symbolic link.
  //IN_MASK_ADD
  // Add (OR) events to watch mask for this pathname if it already
  // exists (instead of replacing mask).
  //IN_ONESHOT
  // Monitor pathname for one event, then remove from watch list.
  //IN_ONLYDIR (since Linux 2.6.15)
  // Only watch pathname if it is a directory.

  int wd = inotify_add_watch(ifd, name.c_str(), mask);
  if(wd == -1) {
    perror("INotify: Add watch failed!");
    return;
  }

  Watch w;
  w.mask = mask;
  w.name = name;
  w.depth = WATCH_SINGLE;
  dirmap[wd] = w;
}

static inline bool isdir(std::string name)
{
  struct stat s;
  stat(name.c_str(), &s);
  return S_ISDIR(s.st_mode);
}

void INotify::addDirectory(std::string name, depth_t depth, uint32_t mask)
{
  DEBUG(inotify, "Adding dir: %s\n", name.c_str());

  int depth_mask = 0;
  if(depth == WATCH_DEEP || depth == WATCH_DEEP_FOLLOW) {
    depth_mask = IN_CREATE; // We need to watch for create in order to catch
    // creation of new subdirs.

    DIR *dir = opendir(name.c_str());
    if(!dir) {
      ERR(inotify, "Could not open directory: %s - %s\n",
          name.c_str(), strerror(errno));
      return;
    }

    struct dirent *dirent;
    while( (dirent = readdir(dir)) != nullptr ) {

      if(std::string(dirent->d_name) == "." ||
         std::string(dirent->d_name) == "..")
        continue;

      if(isdir(name+"/"+dirent->d_name))
        addDirectory(name+"/"+dirent->d_name, depth, mask);
    }

    closedir(dir);
  }

  int wd = inotify_add_watch(ifd, name.c_str(), mask | IN_ONLYDIR | depth_mask);
  if(wd == -1) {
    ERR(inotify, "INotify: Add watch failed on %s\n", name.c_str());
    return;
  }

  Watch w;
  w.mask = mask;
  w.name = name;
  w.depth = depth;
  dirmap[wd] = w;
}

void INotify::remove(int wd)
{
  if(inotify_rm_watch(ifd, wd) == -1) {
    perror("inotify_rm_watch failed");
    return;
  }
  dirmap.erase(wd);
}

void INotify::remove(std::string name)
{
  std::map<int, Watch>::iterator i = dirmap.begin();
  while(i != dirmap.end()) {
    Watch w = i->second;
    if(w.name == name) this->remove(i->first);
    i++;
  }
}

void INotify::readEvents()
{
  size_t size = 64;
  char *buf = (char*)malloc(size);

  ssize_t r;
  while( ((r = read(ifd, buf, size)) == -1 && errno == EINVAL) || r == 0 ) {
    //    fprintf(stderr, "Doubling buffer size: %d\n", size);
    size *= 2;
    buf = (char*)realloc(buf, size);
  }

  int p = 0;
  while(p < r) {
    struct inotify_event *event = (struct inotify_event*)(buf + p);
    p += sizeof(struct inotify_event) + event->len;

    /*
    switch(event.mask) {
    case IN_IGNORED:
      //Watch was removed explicitly (inotify_rm_watch(2)) or automatically 
      // (file was deleted, or file system was unmounted).
    case IN_ISDIR:
      //Subject of this event is a directory.
    case IN_Q_OVERFLOW:
      //Event queue overflowed (wd is -1 for this event).
    case IN_UNMOUNT:
      //File system containing watched object was unmounted.
      break;
    default:
      break;
    }
    */

    // TODO: We need to figure out what the new filename/destination is...
    if(TEST(event->mask, IN_MOVE_SELF)) dirmap[event->wd].name = "????????";

    if(dirmap[event->wd].depth == WATCH_DEEP_FOLLOW && 
       TEST(event->mask, IN_CREATE) && 
       TEST(event->mask, IN_ISDIR))
      addDirectory(dirmap[event->wd].name + "/" + event->name,
                   dirmap[event->wd].depth, dirmap[event->wd].mask);

    if(TEST(event->mask, IN_CREATE) &&
       !TEST(dirmap[event->wd].mask, IN_CREATE)) {
      // Ignore this event, it was not requested by the user.
    } else {
      eventlist.push_back(INotify::Event(event, dirmap[event->wd].name));
    }

    if(TEST(event->mask, IN_IGNORED)) dirmap.erase(event->wd);
  }

  free(buf);
}

bool INotify::hasEvents()
{
  readEvents();
  return eventlist.size() > 0;
}

INotify::Event INotify::getNextEvent()
{
  readEvents();
  if(eventlist.size() == 0) return Event(nullptr, "");
  Event e = eventlist.front();
  eventlist.pop_front();
  return e;
}

void INotify::clear()
{
  if(ifd != -1) close(ifd);
  ifd = inotify_init1(O_NONBLOCK); 
  if(ifd == -1) {
    perror("Inotify init failed.\n");
  }
}

#ifdef TEST_INOTIFY
//deps: debug.cc log.cc mutex.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include "test.h"

#include <stdio.h>

#define _BASEDIR "/tmp"
#define _DIR _BASEDIR"/inotify_test_dir"
#define _FILE _BASEDIR"/inotify_test"

TEST_BEGIN;

debug_parse("+all");

INotify inotify;

// Create file
FILE *fp = fopen(_FILE, "w");
if(!fp) TEST_FATAL("Unable to write to the file");
fprintf(fp, "something");
fclose(fp);

inotify.addFile(_FILE);

TEST_MSG("Positive tests on file watch.");

// Append to file
fp = fopen(_FILE, "r");
if(!fp) TEST_FATAL("Unable to read from the file");
TEST_TRUE(inotify.hasEvents(), "Test if the open event was triggered.");
TEST_TRUE(inotify.getNextEvent().isOpenEvent(), "Test if the event was an open event.");

char buf[32];
fread(buf, sizeof(buf), 1, fp);
TEST_TRUE(inotify.hasEvents(), "Test if the read event was triggered.");
TEST_TRUE(inotify.getNextEvent().isAccessEvent(), "Test if the event was a access event.");

fclose(fp);
TEST_TRUE(inotify.hasEvents(), "Test if the close event was triggered.");
TEST_TRUE(inotify.getNextEvent().isCloseNoWriteEvent(), "Test if the event was a close-nowrite event.");


// Append to file
fp = fopen(_FILE, "a");
if(!fp) TEST_FATAL("Unable to write to the file");
TEST_TRUE(inotify.hasEvents(), "Test if the open event was triggered.");
TEST_TRUE(inotify.getNextEvent().isOpenEvent(), "Test if the event was an open event.");

fprintf(fp, "else"); fflush(fp);
TEST_TRUE(inotify.hasEvents(), "Test if the append event was triggered.");
TEST_TRUE(inotify.getNextEvent().isModifyEvent(), "Test if the event was a modified event.");

fclose(fp);
TEST_TRUE(inotify.hasEvents(), "Test if the close event was triggered.");
TEST_TRUE(inotify.getNextEvent().isCloseWriteEvent(), "Test if the event was a close event.");


// Overwrite file
fp = fopen(_FILE, "w");
if(!fp) TEST_FATAL("Unable to write to the file");

// Open for write initially empties the file content, thus provoking a changed event.
TEST_TRUE(inotify.hasEvents(), "Test if the modified event was triggered.");
TEST_TRUE(inotify.getNextEvent().isModifyEvent(), "Test if the event was a modified event.");

TEST_TRUE(inotify.hasEvents(), "Test if the open event was triggered.");
TEST_TRUE(inotify.getNextEvent().isOpenEvent(), "Test if the event was an open event.");

fprintf(fp, "else"); fflush(fp);
TEST_TRUE(inotify.hasEvents(), "Test if the write event was triggered.");
TEST_TRUE(inotify.getNextEvent().isModifyEvent(), "Test if the event was a modified event.");

fclose(fp);
TEST_TRUE(inotify.hasEvents(), "Test if the close event was triggered.");
TEST_TRUE(inotify.getNextEvent().isCloseWriteEvent(), "Test if the event was a close event.");

// Rename file
rename(_FILE, _FILE"2");
TEST_TRUE(inotify.hasEvents(), "Test if the rename event was triggered.");
TEST_TRUE(inotify.getNextEvent().isMoveSelfEvent(), "Test if the event was a move self event.");

// Delete file
unlink(_FILE"2");

// Unlink initially counts down the link counter, which provokes an attributes changed event.
TEST_TRUE(inotify.hasEvents(), "Test if the delete event was triggered.");
TEST_TRUE(inotify.getNextEvent().isAttributeChangeEvent(), "Test if the event was an attribute change event.");

// Since the linkcount should now be zero, the file should be deleted.
TEST_TRUE(inotify.hasEvents(), "Test if the delete event was triggered.");
TEST_TRUE(inotify.getNextEvent().isDeleteSelfEvent(), "Test if the event was a delete self event.");

// Watch is removed upon delete.
//inotify.remove(_FILE);

TEST_TRUE(inotify.hasEvents(), "Test if the delete event triggered an ignored event.");
TEST_TRUE(inotify.getNextEvent().isIgnoredEvent(), "Test if the event was an ignored event.");

// Create file again
fp = fopen(_FILE, "w");
if(!fp) TEST_FATAL("Unable to write to the file");
fprintf(fp, "something");
fclose(fp);

inotify.addFile(_FILE);
inotify.remove(_FILE);

TEST_TRUE(inotify.hasEvents(), "Test if the call to remove triggered an ignored event.");
TEST_TRUE(inotify.getNextEvent().isIgnoredEvent(), "Test if the event was an ignored event.");

// Delete file
unlink(_FILE);
inotify.getNextEvent();
TEST_FALSE(inotify.hasEvents(), "Test if the delete event was ignored.");

TEST_FALSE(inotify.hasEvents(), "Test if the event queue is now empty.");

TEST_MSG("Positive tests on directory watch.");

if(mkdir(_DIR, 0777) == -1) TEST_FATAL("Could not create test dir.");
inotify.addDirectory(_DIR, WATCH_DEEP_FOLLOW);

// Create file again
fp = fopen(_DIR"/file1", "w");
if(!fp) TEST_FATAL("Unable to write to the file");
fprintf(fp, "something");
fclose(fp);

TEST_TRUE(inotify.hasEvents(), "Test if the file creation triggered events.");
TEST_TRUE(inotify.getNextEvent().isCreateEvent(), "...");
TEST_TRUE(inotify.getNextEvent().isOpenEvent(), "...");
TEST_TRUE(inotify.getNextEvent().isModifyEvent(), "...");
TEST_TRUE(inotify.getNextEvent().isCloseWriteEvent(), "...");

rename(_DIR"/file1", _DIR"/file2");
TEST_TRUE(inotify.hasEvents(), "Test if the file renaming triggered events.");
TEST_TRUE(inotify.getNextEvent().isMovedFromEvent(), "...");
TEST_TRUE(inotify.getNextEvent().isMovedToEvent(), "...");

unlink(_DIR"/file2");

if(mkdir(_DIR"/dir", 0777) == -1) TEST_FATAL("Could not create test dir.");

if(mkdir(_DIR"/dir/anotherdir", 0777) == -1) TEST_FATAL("Could not create test dir.");

while(inotify.hasEvents()) inotify.getNextEvent();

rmdir(_DIR"/dir/anotherdir");

rmdir(_DIR"/dir");

rmdir(_DIR);

while(inotify.hasEvents()) inotify.getNextEvent();

TEST_END;

#endif/*TEST_INOTIFY*/

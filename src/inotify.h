/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            inotify.h
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
#ifndef __PRACRO_INOTIFY_H__
#define __PRACRO_INOTIFY_H__

#include <sys/types.h>
#include <sys/inotify.h>

#include <string>
#include <map>
#include <list>

typedef enum {
  WATCH_SINGLE, // Watch only the specified directory.
  WATCH_DEEP, // Watch all current subdirs as well
  WATCH_DEEP_FOLLOW // Watch all current subdir as well as subdirs 
                    // being created after the watch is initiated
} depth_t;


class INotify {
public:
  class Event {
  public:
    Event(struct inotify_event *event, std::string name);

    bool isAttributeChangeEvent();
    bool isCreateEvent();
    bool isOpenEvent();
    bool isCloseEvent();
    bool isCloseWriteEvent();
    bool isCloseNoWriteEvent();
    bool isModifyEvent();
    bool isAccessEvent();
    bool isDeleteEvent();
    bool isDeleteSelfEvent();
    bool isIgnoredEvent();
    bool isMoveSelfEvent();
    bool isMovedFromEvent();
    bool isMovedToEvent();

    bool isDir();

    std::string name();
    std::string file();
    uint32_t mask();
    std::string maskstr();

  private:
    std::string _name;
    std::string _file;
    uint32_t _mask;
  };

  INotify();
  ~INotify();

  void addFile(std::string name, uint32_t mask = IN_ALL_EVENTS);

  /**
   * WARNING: If a directory is added with WATCH_DEEP_FOLLOW, newly 
   * created folders will not be added to the watch before the next call
   * to hasEvents or getNextEvent. thie means that any files created in
   * that folder prior to htese calls will not be caught. This will need
   * to be done mnually by a recursive scan.
   */
  void addDirectory(std::string name,
                    depth_t depth = WATCH_SINGLE,
                    uint32_t mask = IN_ALL_EVENTS);
  void remove(std::string name);
  void remove(int fd);

  bool hasEvents();
  Event getNextEvent();

  void clear();

private:
  class Watch {
  public:
    std::string name;
    uint32_t mask;
    depth_t depth;
  };

  void readEvents();

  int ifd;
  std::map<int, Watch> dirmap;
  std::list<Event> eventlist;
};

#endif/*__PRACRO_INOTIFY_H__*/

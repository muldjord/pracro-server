/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            entitylist.h
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
#ifndef __PRACRO_ENTITYLIST_H__
#define __PRACRO_ENTITYLIST_H__

#include <map>
#include <string>

#include "versionstr.h"
#include "mutex.h"

#include "inotify.h"

#include "exception.h"

/**
 * The Items contained in the EntityList.
 */
typedef std::multimap<VersionStr, std::string> EntityListItem;

/**
 * The EntityList class is intended for entity file caching, so that all entitys
 * do not need to be parsed on each entity query.
 * It builds a list of entitys and versions based on the informations read from
 * the EntityHeaderParser.
 * This means that just because a entity gets into the list doesn't means that it
 * will validate as a correct entity (not even nessecarily correct XML).
 */
class EntityList : public std::map<std::string, EntityListItem > {
public:
  /**
   * Constructor.
   * @param entitypath A std::string containing the path in which we should look
   * for xml files.
   */
  EntityList(std::string entitypath, std::string entityname);
  virtual ~EntityList();

  /**
   * Convenience method, to gain the filename of the latest version of a given entity.
   * This method throws an Exception if the entity does not exist in the tree.
   * @param entity A std::string containing the name of the wanted entity.
   * @return A std::string containing the file containing the entity with full path
   * included.
   */
  std::string getLatestVersion(std::string entity);

protected:
  void rescan();
  void insertEntity(std::string entity, std::string version, std::string file);

private:
  virtual void addFile(std::string file) = 0;

  bool removeFile(std::string file);
  void updateFile(std::string file);
  void updateList();

  Mutex mutex;
  INotify inotify;
  
  std::string entityname;
  std::string entitypath;
};

#endif/*__PRACRO_ENTITYLIST_H__*/

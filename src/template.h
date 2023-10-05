/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            template.h
 *
 *  Mon May 12 10:42:39 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
#ifndef __PRACRO_TEMPLATE_H__
#define __PRACRO_TEMPLATE_H__

#include <vector>
#include <string>
#include <map>

typedef std::map< std::string, std::string > attr_t;

class Widget {
public:
  std::vector< Widget > widgets;
  attr_t attributes;
};

class Script {
public:
  attr_t attributes;
  std::string code;
};

class Map {
public:
  attr_t attributes;
};
typedef std::vector< Map > maps_t;

class Query {
public:
  attr_t attributes;
};

class Resume {
public:
  attr_t attributes;
};

class Macro {
public:
  std::vector< Query > queries;
  maps_t maps;
  std::vector< Script > scripts;
  std::vector< Script > resume_scripts;
  std::vector< Script > commit_scripts;
  Widget widgets;
  Resume resume;

  bool isHeader;
  bool isStatic;
  bool isCompact;
  bool isImportant;

  std::string name;
  std::string version;
  std::string caption;
  std::string requires;
  std::string ttl;
};

class Template {
public:
  std::vector< Script > scripts;

  std::vector< Macro > macros;

  std::string name;
  std::string title;
  std::string version;
};

class Course {
public:
  std::vector< Template > templates;

  std::string name;
  std::string title;
  std::string version;
};

#endif/*__PRACRO_TEMPLATE_H__*/

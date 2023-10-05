/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            environment.h
 *
 *  Tue Jan  5 11:41:23 CET 2010
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
#ifndef __PRACRO_ENVIRONMENT_H__
#define __PRACRO_ENVIRONMENT_H__

#include "database.h"
#include "artefact.h"
#include "connectionpool.h"
#include "session.h"
#include "courselist.h"
#include "templatelist.h"
#include "macrolist.h"

class Environment {
public:
  Environment();
  ~Environment();

  ConnectionPool<Database*> dbpool;
  ConnectionPool<Artefact*> atfpool;
  Sessions sessions;
  MacroList macrolist;
  TemplateList templatelist;
  CourseList courselist;
};

#endif/*__PRACRO_ENVIRONMENT_H__*/

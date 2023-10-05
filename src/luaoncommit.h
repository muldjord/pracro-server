/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            luaoncommit.h
 *
 *  Thu Jan 12 08:38:02 CET 2012
 *  Copyright 2012 Bent Bisballe Nyeng
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
#ifndef __PRACRO_LUAONCOMMIT_H__
#define __PRACRO_LUAONCOMMIT_H__

#include "luascript.h"

#include "transaction.h"
#include <string>

class LUAOnCommit : public LUAScript {
public:
  LUAOnCommit() {}
  LUAOnCommit(Transaction &transaction, Commit &commit);
  virtual ~LUAOnCommit() {}

  const char *name() { return "lua on commit"; }
};

#endif/*__PRACRO_LUAONCOMMIT_H__*/

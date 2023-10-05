/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            daemon.h
 *
 *  Thu Jun  9 10:27:59 CEST 2005
 *  Copyright  2005 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __DAEMON_H__
#define __DAEMON_H__

#include <sys/types.h>

#include <string>

class Daemon {
public:
  Daemon();
  virtual ~Daemon();
  
  /**
   * Use NOBODY_GROUP and NOBODY_USER if no privileges are needed to run.
   */
  int run(const char* user, const char* group, bool detach = true,
          std::string pidfile = "");

private:
  virtual int daemon_main() = 0;
  
};

#endif/*__DAEMON_H__*/

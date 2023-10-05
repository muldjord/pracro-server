/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            journal_commit.h
 *
 *  Tue Mar 18 11:09:59 CET 2008
 *  Copyright 2008 Bent Bisballe Nyeng and Peter Skaarup
 *  deva@aasimon.org and piparum@piparum.dk
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
#ifndef __PRACRO_JOURNAL_COMMIT_H__
#define __PRACRO_JOURNAL_COMMIT_H__

#include <unistd.h>

int journal_commit(const char *cpr, const char *user,
                   const char *addr, unsigned short int port,
                   const char *buf, size_t size);

#endif/*__PRACRO_JOURNAL_COMMIT_H__*/

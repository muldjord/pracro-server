/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            semaphore.cc
 *
 *  Sat Oct  8 17:44:13 CEST 2005
 *  Copyright  2005 Bent Bisballe Nyeng
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
#include "semaphore.h"

Semaphore::Semaphore()
{
  sem_init(&semaphore, 0, 0);
}

Semaphore::~Semaphore()
{
  sem_destroy(&semaphore);
}

void Semaphore::post()
{
  sem_post(&semaphore);
}

void Semaphore::wait()
{
  sem_wait(&semaphore);
}

#ifdef TEST_SEMAPHORE
//deps:
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include <test.h>

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_SEMAPHORE*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            connectionpool.cc
 *
 *  Wed Dec 16 12:20:44 CET 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#include "connectionpool.h"

//
// Implementation is in the header file.
//

#ifdef TEST_CONNECTIONPOOL
//deps: mutex.cc semaphore.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include <test.h>
#include <unistd.h>
#include <pthread.h>

static void* thread_run(void *data)
{
  ConnectionPool<int> *pool = (ConnectionPool<int>*)data;
  
  int db1 = pool->borrow();
  int db2 = pool->borrow();
  int db3 = pool->borrow();
  int db4 = pool->borrow();

  usleep(100);

  pool->giveBack(db1);
  pool->giveBack(db2);
  pool->giveBack(db3);
  pool->giveBack(db4);

  return nullptr;
}

static void* thread_run_clear_test(void *data)
{
  ConnectionPool<int> *pool = (ConnectionPool<int>*)data;
  pool->giveBack(1);
  pool->giveBack(2);

  sleep(1);

  pool->giveBack(3);
  pool->giveBack(4);

  return nullptr;
}

TEST_BEGIN;

ConnectionPool<int> pool;

int db1 = 1;
int db2 = 2;
int db3 = 3;
int db4 = 4;

pool.add(db1);
pool.add(db2);
pool.add(db3);
pool.add(db4);

TEST_TRUE(pool.testFree(db1), "Testing if db1 is free.");
TEST_TRUE(pool.testFree(db2), "Testing if db2 is free.");
TEST_TRUE(pool.testFree(db3), "Testing if db3 is free.");
TEST_TRUE(pool.testFree(db4), "Testing if db4 is free.");
TEST_EQUAL(pool.numFree(), 4, "Testing number of free databases.");

int b_db1 = pool.borrow();
TEST_FALSE(pool.testFree(b_db1), "Testing if borrowed db is free.");

int b_db2 = pool.borrow();
TEST_NOTEQUAL(b_db1, b_db2, "Testing if borrowed db is unique.");

pool.giveBack(b_db1);
TEST_TRUE(pool.testFree(b_db1), "Testing if returned db is free.");
pool.giveBack(b_db2);

TEST_EQUAL(pool.numFree(), 4, "Testing number of free databases.");

{
  pthread_attr_t attr;
  pthread_t tid;
  pthread_attr_init(&attr);
  pthread_create(&tid, &attr, thread_run, &pool);
  
  while(pool.numFree() > 0) { usleep(10); }
  
  int b_db3 = pool.borrow();
  TEST_FALSE(pool.testFree(b_db3), "Testing if returned db is free (semaphore test).");
  pool.giveBack(db3);
  
  pthread_join(tid, nullptr);
  pthread_attr_destroy(&attr);
}

TEST_EQUAL(pool.numFree(), 4, "Testing if all database are now available again");

{
  TEST_EQUAL(pool.numFree(), 4, "Testing if autoborrower has not yet borrowed a db.");
  AutoBorrower<int> b(pool);
  TEST_EQUAL(pool.numFree(), 3, "Testing if autoborrower has borrowed a db.");
  TEST_FALSE(pool.testFree(b.get()), "Testing if the autoborrowed db is actually taken.");
}
TEST_EQUAL(pool.numFree(), 4, "Testing if autoborrower has returned the db.");

// Force remove all elements.
pool.borrow();
pool.clear(true);
TEST_EQUAL(pool.numFree(), 0, "Testing number of free databases.");

// Add them again.
pool.add(db1);
pool.add(db2);
pool.add(db3);
pool.add(db4);
TEST_EQUAL(pool.numFree(), 4, "Testing number of free databases.");

pool.borrow();
pool.borrow();
pool.borrow();
pool.borrow();

{
  pthread_attr_t attr;
  pthread_t tid;
  pthread_attr_init(&attr);
  pthread_create(&tid, &attr, thread_run_clear_test, &pool);
  pool.clear(false);
  pthread_join(tid, nullptr);
  pthread_attr_destroy(&attr);
}

TEST_END;

#endif/*TEST_CONNECTIONPOOL*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            connectionpool.h
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
#ifndef __PRACRO_CONNECTIONPOOL_H__
#define __PRACRO_CONNECTIONPOOL_H__

#include <list>

#include "mutex.h"
#include "semaphore.h"

template <class T>
class ConnectionPool {
public:
  void add(T t);
  void remove(T t);

  bool testFree(T t);
  int numFree();

  T borrow();
  void giveBack(T t);

  std::list<T> clear(bool force = true);

private:
  bool contains(std::list<T> &list, T t);

  Semaphore semaphore;
  Mutex mutex;
  std::list<T> active;
  std::list<T> passive;
};

template <class T>
class AutoBorrower {
public:
  AutoBorrower(ConnectionPool<T> &pool);
  ~AutoBorrower();

  T get();

private:
  ConnectionPool<T> &pool;
  T t;
};


//
// Implementation is below
//

template <class T>
void ConnectionPool<T>::add(T t)
{
  MutexAutolock lock(mutex);

  passive.push_back(t);
  semaphore.post();

}

template <class T>
bool ConnectionPool<T>::contains(std::list<T> &list, T t)
{
  typename std::list<T>::iterator i = list.begin();
  while(i != list.end()) {
    if(*i == t) return true;
    i++;
  }

  return false;
}

template <class T>
void ConnectionPool<T>::remove(T t)
{
  MutexAutolock lock(mutex); 

  if(contains(passive, t)) {
    semaphore.post();
    passive.remove(t);
  }

}

template <class T>
bool ConnectionPool<T>::testFree(T t)
{
  bool testfree = false;

  MutexAutolock lock(mutex);
  testfree = contains(passive, t);

  return testfree;
}

template <class T>
int ConnectionPool<T>::numFree()
{
  int num;

  MutexAutolock lock(mutex);
  num = passive.size();

  return num;
}

template <class T>
T ConnectionPool<T>::borrow()
{
  T t;

  semaphore.wait();

  {
    MutexAutolock lock(mutex);

    t = passive.front();
    passive.remove(t);
    active.push_back(t);
  }

  return t;
}

template <class T>
void ConnectionPool<T>::giveBack(T t)
{
  MutexAutolock lock(mutex);

  if(contains(active, t)) {
    active.remove(t);
    passive.push_back(t);
    semaphore.post();
  }
}

template <class T>
std::list<T> ConnectionPool<T>::clear(bool force)
{
  typename std::list<T> lst;

  if(force == false) {
    size_t num = 0;
    {
      MutexAutolock lock(mutex);
      num = active.size() + passive.size();
    }

    while(num) {
      borrow();
      num--;
    }
  }

  {
    MutexAutolock lock(mutex);

    typename std::list<T>::iterator i;

    i = active.begin();
    while(i != active.end()) {
      lst.push_back(*i);
      //      i = active.erase(i);
      semaphore.post();
      i++;
    }
    active.clear();

    i = passive.begin();
    while(i != passive.end()) {
      lst.push_back(*i);
      //      i = passive.erase(i);
      i++;
    }
    passive.clear();
  }

  return lst;
}

template <class T>
AutoBorrower<T>::AutoBorrower(ConnectionPool<T> &p)
  : pool(p)
{
  t = pool.borrow();
}

template <class T>
AutoBorrower<T>::~AutoBorrower()
{
  pool.giveBack(t);
}

template <class T>
T AutoBorrower<T>::get()
{
  return t;
}

#endif/*__PRACRO_CONNECTIONPOOL_H__*/

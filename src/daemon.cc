/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            daemon.cc
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

#include "daemon.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// For getgrent and getgrent
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

// For strcmp
#include <string.h>

// Resolve a group's name or id into a numeric gid
static gid_t get_gid(const char *grp)
{
  char *eptr;
  gid_t xid = strtoul(grp, &eptr, 10);
  struct group *gr;
  errno = 0;      // Per getgrnam(3) and getgrgid(3) manual page
  if(!*eptr)
    gr = getgrgid(xid);
  else
    gr = getgrnam(grp);
  return !gr ? 0 : gr->gr_gid;
}

// Resolve a user's name or id into a numeric uid with associated gid
static uid_t get_uid(const char *usr, gid_t *gid)
{
  char *eptr;
  uid_t xid = strtoul(usr, &eptr, 10);
  struct passwd *pw;
  errno = 0;      // Per getpwnam(3) and getpwuid(3) manual page
  if(!*eptr)
    pw = getpwuid(xid);
  else
    pw = getpwnam(usr);
  if(!pw)
    return 0;
  if(!gid)
    *gid = pw->pw_gid;
  return pw->pw_uid;
}

Daemon::Daemon()
{}

Daemon::~Daemon()
{}

int Daemon::run(const char *user, const char* group, bool detach,
                std::string pidfile)
{
  // Fetch user and group IDs
  gid_t gid = 0;
  uid_t uid = 0;
  if(user && *user) {
    uid = get_uid(user, &gid);
    if(errno) {
      fprintf(stderr, "Could resolve user \"%s\"", user);
      perror("");
    }
  }

  if(group && *group) {
    gid = get_gid(group);
    if(errno) {
      fprintf(stderr, "Could not resolve group \"%s\"", group);
      perror("");
    }
  }

  umask(0);

  /*
  if(pidfile != "" ) {
    FILE *fp = fopen(pidfile.c_str(), "r");
    if(fp != nullptr) {
      fclose(fp);
      fprintf(stderr, "Could not write pid file \"%s\""
              " - file already exists.\n",
              pidfile.c_str());
      return -1;
    }
  }
  */

  if(detach) {
    if(daemon(0, 0) == -1) { 
      perror("");
      return -1; 
    }
  }

  if(pidfile != "" ) {
    pid_t pid = getpid();

    FILE *fp = fopen(pidfile.c_str(), "w");
    if(fp == nullptr) {
      fprintf(stderr, "Could not write pid file \"%s\"", pidfile.c_str());
      perror("");
      return -1;
    } else {
      fprintf(fp, "%lu", (unsigned long int)pid);
      fclose(fp);
      /*
      fprintf(stderr, "Wrote pid %lu to file \"%s\"",
              (unsigned long int)pid, pidfile.c_str());
      */
    }
  }

  if(gid) {
    // Switch to given group
    if(setgid(gid) != 0) {
      fprintf(stderr, "Failed to change to group \"%s\" (gid: %d).\n",
              group, gid);
      perror("");
      fprintf(stderr, "Runnning daemon as current group\n");
    }
  }
    
  if(uid) {
    // Switch to given user
    if(setuid(uid) != 0) {
      fprintf(stderr, "Failed to change to user \"%s\" (uid: %d).\n",
              user, uid);
      perror("");
      fprintf(stderr, "Runnning daemon as current user\n");
    }
  }

  setsid();

  signal (SIGTERM, SIG_IGN);

  //signal (SIGHUP, SIG_IGN);

  // Don't disable Ctrl+c when running in foreground.
  //if(detach) signal (SIGINT, SIG_IGN);

  int ret = daemon_main();

  if(pidfile != "") {
    unlink(pidfile.c_str());
  }

  return ret;
}

#ifdef TEST_DAEMON
//deps:
//cflags:
//libs:
#include <test.h>

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

TEST_END;

#endif/*TEST_DAEMON*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pracrod.cc
 *
 *  Wed Aug 22 12:15:59 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
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
// For ETC
#include <config.h>

#include "daemon.h"

// For setuid and setgid
#include <sys/types.h>
#include <unistd.h>

// For waitpid
#include <sys/wait.h>

// For signal
#include <signal.h>

// For errno and strerror
#include <errno.h>

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

// For getpwent and getgrent
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

// For getopt_long and friends
#include <getopt.h>

#include "configurationparser.h"
#include "configuration.h"

#include "server.h"

#include "tcpsocket.h"

#include <hugin.hpp>

static const char version_str[] =
"Pracro server v" VERSION "\n"
;

static const char copyright_str[] =
"Copyright (C) 2006-2012 Bent Bisballe Nyeng - Aasimon.org.\n"
"This is free software.  You may redistribute copies of it under the terms of\n"
"the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
"There is NO WARRANTY, to the extent permitted by law.\n"
"\n"
"Written by Bent Bisballe Nyeng (deva@aasimon.org)\n"
;

static const char usage_str[] =
"Usage: %s [options]\n"
"Options:\n"
"  -c, --config file       Read configfile from 'file'\n"
"  -f, --foreground        Run in foreground mode (non-background mode)\n"
"  -u, --user user         Run as 'user' (overrides the configfile)\n"
"  -g, --group group       Run as 'group' (overrides the configfile)\n"
"  -x, --xml-basedir d     Use 'd' as basedir for finding template- and macro-files (default " XML ").\n"
"  -v, --version           Print version information and exit.\n"
"  -h, --help              Print this message and exit.\n"
"  -D, --debug ddd         Enable debug messages on 'ddd'; see documentation for details\n"
"  -d  --database db       Use db as the database backend. Can be one of pgsql or testdb (default pgsql).\n"
"  -s, --ssl               Enable ssl encryption with key configured in config file.\n"
"  -P, --pidfile file      Write pid to file.\n"
"  -L, --logfile file      Write output to file, instead of stderr.\n"
"  -S, --session-path dir  Use dir as the path for active session storage.\n"
;

ConfigurationParser *configparser = nullptr;

bool pracro_is_running = true;

void ctrl_c(int)
{
  //  printf("Ctrl+c\n");
  pracro_is_running = false;
}

void childwait(int)
{
  //  printf("childwait\n");

  pid_t pid;
  while((pid = waitpid(-1, nullptr, WNOHANG)) > 0) {
    //    printf("\tOne down!\n");
  }
}

void reload(int)
{
  printf("Reopening logfile...\n");
  if(hugin_reopen_log() != HUG_STATUS_OK) {
    fprintf(stderr, "Could not reopen logfile!\n");
    return;
  }
  DEBUG(pracro, "Reopened log\n");
}

class PracroDaemon : public Daemon {
private:
  int daemon_main();
};

int PracroDaemon::daemon_main()
{
  // Activate the server main loop.
  server();

  return 0;
}

// Demo CERT, DO NOT USE IN PRODUCTION!
#define CERT "\
-----BEGIN CERTIFICATE-----\n\
MIICFTCCAX6gAwIBAgIBAjANBgkqhkiG9w0BAQUFADBVMRswGQYDVQQKExJBcGFj\n\
aGUgSFRUUCBTZXJ2ZXIxIjAgBgNVBAsTGUZvciB0ZXN0aW5nIHB1cnBvc2VzIG9u\n\
bHkxEjAQBgNVBAMTCWxvY2FsaG9zdDAeFw0wNzA2MjEwODE4MzZaFw0wODA2MjAw\n\
ODE4MzZaMEwxGzAZBgNVBAoTEkFwYWNoZSBIVFRQIFNlcnZlcjEZMBcGA1UECxMQ\n\
VGVzdCBDZXJ0aWZpY2F0ZTESMBAGA1UEAxMJbG9jYWxob3N0MIGfMA0GCSqGSIb3\n\
DQEBAQUAA4GNADCBiQKBgQDWTACKSoxd5cL06w7RtPIhFqY1l3UE/aRGmPmh8gEo\n\
w3zNf+gWxco2yjQgBTQhGww1ybOsAUtXPIsUOSFAGvPUKJZf8ibZMiJEzl2919uz\n\
IcV9+cUm7k3jFPQx4ALQEalbV++o/lfT5lhgsSiH1t1eln2omVrGCjI/1HeYrw7X\n\
owIDAQABMA0GCSqGSIb3DQEBBQUAA4GBALVFzprK6rYkWVZZZwq85w2lCYJpEl9a\n\
66IMzIwNNRfyZMoc9D9PSwsXKYfYOg1RpMt7RhWT/bpggGlsFqctsAgJSv8Ol5Cz\n\
DqTXhpV+8WOG6l4xDYZz3U3ajiu2jth2+aaMuWKy9Wkr8bzHGDufltToLalucne2\n\
npM7yCJ83Ana\n\
-----END CERTIFICATE-----"

// Demo KEY, DO NOT USE IN PRODUCTION!
#define KEY "\
-----BEGIN RSA PRIVATE KEY-----\n\
MIICXAIBAAKBgQDWTACKSoxd5cL06w7RtPIhFqY1l3UE/aRGmPmh8gEow3zNf+gW\n\
xco2yjQgBTQhGww1ybOsAUtXPIsUOSFAGvPUKJZf8ibZMiJEzl2919uzIcV9+cUm\n\
7k3jFPQx4ALQEalbV++o/lfT5lhgsSiH1t1eln2omVrGCjI/1HeYrw7XowIDAQAB\n\
AoGANUXHjJljs6P+hyw4DuHQn3El+ISiTo9PW02EIUIsD5opWFzHsYGR93Tk6GDi\n\
yKgUrPprdAMOW61tVaWuImWQ32R2xyrJogjGYo9XE2xAej9N37jM0AGBtn/vd4Dr\n\
LsYfpjNaM3gqIChD73iYfO+CrNbdLqTxIdG53g/u05GJ4cECQQD0vMm5+a8N82Jb\n\
oHJgE2jb83WqaYBHe0O03ujtiq3+hPZHoVV3iJWmA/aMlgdtunkJT3PdEsVfQNkH\n\
fvzR9JhbAkEA4CiZRk5Gcz7cEqyogDTMQYtmrE8hbgofISLuz1rpTEzd8hFAcerU\n\
nuwFIT3go3hO7oIHMlKU1H5iT1BsFvegWQJBAOSa6A+5A+STIKAX+l52Iu+5tYKN\n\
885RfMgZpBgm/yoMxwPX1r7GLYsajpV5mszLbz3cIo0xeH3mVBOlccEoqZsCQECP\n\
8PWq/eebp09Jo46pplsKh5wBfqNvDuBAa4AVszRiv1pFVcZ52JudZyzX4aezsyhH\n\
E0OPPYamkDI/+6Hx2KECQHF9xV1XatyXuFmfRAInK2BtfGY5UIvJaLxVD3Z1+i6q\n\
/enz7/wUwvC6G4FSWNMYgAYJOfwZ3BerdkqcRNxyR/Q=\n\
-----END RSA PRIVATE KEY-----"

int main(int argc, char *argv[])
{
  const char *hugin_filter = "+all";
  const char *logfile = nullptr;
  unsigned int hugin_flags = 0;

  int c;
  char *configfile = nullptr;
  char *user = nullptr;
  char *group = nullptr;
  bool foreground = false;
  char *xml_basedir = nullptr;
  std::string database;
  std::string pidfile;
  std::string sessionpath;

  int option_index = 0;
  while(1) {
    //    int this_option_optind = optind ? optind : 1;
    static struct option long_options[] = {
      {"foreground", no_argument, 0, 'f'},
      {"config", required_argument, 0, 'c'},
      {"user", required_argument, 0, 'u'},
      {"group", required_argument, 0, 'g'},
      {"help", no_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {"xml-basedir", required_argument, 0, 'x'},
      {"debug", required_argument, 0, 'D'},
      {"database", required_argument, 0, 'd'},
      {"ssl", no_argument, 0, 's'},
      {"pidfile", required_argument, 0, 'P'},
      {"logfile", required_argument, 0, 'L'},
      {"session-path", required_argument, 0, 'S'},
      {0, 0, 0, 0}
    };
    
    c = getopt_long (argc, argv, "D:hvfc:u:g:x:d:sL:P:S:",
                     long_options, &option_index);
    
    if (c == -1)
      break;

    switch(c) {
    case 'd':
      database = optarg;
      break;

    case 'c':
      configfile = strdup(optarg);
      break;

    case 'f':
      foreground = true;
      break;

    case 'u':
      user = strdup(optarg);
      break;

    case 'g':
      group = strdup(optarg);
      break;

    case 'x':
      xml_basedir = strdup(optarg);
      break;

    case 'P':
      pidfile = optarg;
      break;

    case 'L':
      hugin_flags |= HUG_FLAG_OUTPUT_TO_FILE;
      logfile = strdup(optarg);
      break;

    case 'S':
      sessionpath = optarg;
      break;

    case 'D':
      hugin_flags |= HUG_FLAG_USE_FILTER;
      hugin_filter = optarg;
      break;

    case 's':
#ifdef WITHOUT_SSL
      ERR(server, "Pracro was not compiled with SSL support!\n");
      return 1;
#else
      Conf::use_ssl = true;
      Conf::ssl_key = KEY;
      Conf::ssl_cert = CERT;
#endif
      break;

    case '?':
    case 'h':
      printf("%s", version_str);
      printf(usage_str, argv[0]);
      return 0;

    case 'v':
      printf("%s", version_str);
      printf("%s", copyright_str);
      return 0;

    default:
      break;
    }
  }

  if(logfile == nullptr) hugin_flags |= HUG_FLAG_OUTPUT_TO_STDOUT;

  hug_status_t status = hug_init(hugin_flags,
                                 HUG_OPTION_FILTER, hugin_filter,
                                 HUG_OPTION_FILENAME, logfile,
                                 HUG_OPTION_END);
  if(status != HUG_STATUS_OK) {
    printf("Error: %d\n", status);
    return 1;
  }

  // Load config
  try {
    if(configfile) configparser = new ConfigurationParser(configfile);
    else configparser = new ConfigurationParser(ETC"/pracrod.conf");
  } catch(ConfigurationParser::ParseException &e) {
    ERR(pracrod, "Config file parse error: %s.\n", e.what());
    return 1;
  } catch(ConfigurationParser::ReadException &e) {
    ERR(pracrod, "Config file read error: %s.\n", e.what());
    return 1;
  }

  if(sessionpath != "") {
    Conf::session_path = sessionpath;
  }

  if(database != "") {
    Conf::database_backend = database;
  }

  if(Conf::database_backend == "testdb") {
    WARN(pracrod, "Test db (memory only db) does not work in plural.\n");
    Conf::database_poolsize = 1;
  }

  if(!user) {
    user = strdup(Conf::server_user.c_str());
  }


  if(!group) {
    group = strdup(Conf::server_group.c_str());
  }

  if(xml_basedir) {
    Conf::xml_basedir = xml_basedir;
  }

  signal(SIGHUP, reload);
  //  signal(SIGCLD, childwait);
  signal(SIGINT, ctrl_c);

  PracroDaemon daemon;
  daemon.run(user, group, !foreground, pidfile);

  // Clean up
  if(configfile) free(configfile);
  if(user) free(user);
  if(group) free(group);

  return 0;
}

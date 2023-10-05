/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            httpd.h
 *
 *  Thu Jun 10 09:05:10 CEST 2010
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
#ifndef __PRACRO_HTTPD_H__
#define __PRACRO_HTTPD_H__

#include <map>
#include <string>

class headers_t : public std::map< std::string, std::string > {
public:
  bool contains(std::string name) {
    return find(name) != end();
  }

  std::string lookup(std::string key, std::string defval = "")
  {
    if(contains(key)) return (*this)[key];
    return defval;
  }
};

struct MHD_Daemon;

class Httpd {
public:
  class Reply {
  public:
    std::string data;
    headers_t headers;
    int status;
  };

  Httpd();
  virtual ~Httpd();

  void listen(unsigned short int port,
              unsigned int cn_limit = 1, unsigned int cn_timeout = 0);
  void stop();
  bool is_running();

  void listen_ssl(unsigned short int port,
                  std::string key, std::string cert,
                  unsigned int cn_limit = 1, unsigned int cn_timeout = 0);
  void stop_ssl();
  bool is_running_ssl();

  virtual void error(const std::string &err) {}

  // The retruned void pointer will be given as an argument to data, complete
  //  and cleanup methods.
  virtual void *begin(const std::string &url,
                      const std::string &method,
                      const std::string &version,
                      headers_t &getargs,
                      headers_t &headers) { return nullptr; }

  // Return false indicates error, and terminates connetion (no reply)
  virtual bool data(void *ptr, const char *data,
                    unsigned int data_size) { return false; }

  // Return false indicates error, and terminates connetion (no reply)
  virtual bool complete(void *ptr, Httpd::Reply &reply) { return false; }

  virtual void cleanup(void *ptr) {}

private:
  struct MHD_Daemon *d;
  struct MHD_Daemon *d_ssl;
};

#endif/*__PRACRO_HTTPD_H__*/

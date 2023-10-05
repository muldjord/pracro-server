/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            httpd.cc
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
#include "httpd.h"

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// For fork
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <microhttpd.h>
#include <time.h>

#include <hugin.hpp>

// for inet_ntop
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
  void *ptr;
  bool firstrun;
  size_t total;
  size_t acc;
} connection_t;

MHD_Result hdit(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
  DEBUG(httpd, "%s: %s\n", key, value);

  headers_t *headers = (headers_t*)cls;

  (*headers)[key] = value;

  return MHD_YES;
}

static MHD_Result request_handler(void *cls,
                           struct MHD_Connection *con,
                           const char *url,
                           const char *method,
                           const char *version,
                           const char *data,
                           size_t *data_size,
                           void **con_cls)
{
  time_t now = time(nullptr);
  DEBUG(httpd, "Request time: %s", ctime(&now));

  void *so;
  so = MHD_get_connection_info(con, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;

  char peeraddr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &((struct sockaddr_in*)so)->sin_addr, peeraddr, sizeof(peeraddr));
  DEBUG(httpd, "peer: %s\n", peeraddr);

  DEBUG(httpd, "request_handler: cls(%p) con(%p) url(%s) method(%s)"
        " version(%s) *con_cls(%p)\n",
        cls, con, url, method, version, *con_cls);
  std::string datastr; datastr.append(data, *data_size);
  DEBUG(httpd, "request_handler: *data_size(%u) data:[%s]\n",
        (unsigned int)*data_size, datastr.c_str());

  MHD_Result ret = MHD_YES;

  Httpd *httpd = (Httpd*)cls;

  if(*con_cls == nullptr) {
    headers_t headers;
    headers_t getargs;

    DEBUG(httpd, "URI: %s\n", url);

    MHD_get_connection_values(con, MHD_HEADER_KIND, &hdit, &headers);
    int n = MHD_get_connection_values(con, MHD_GET_ARGUMENT_KIND, &hdit, &getargs);
    DEBUG(httpd, "Num args: %d\n", n);

    connection_t* c = new connection_t;

    c->firstrun = true;
    c->total = 0;
    if(headers.find("Content-Length") != headers.end()) {
      c->total = atoi(headers["Content-Length"].c_str());
    }
    c->acc = 0;
    c->ptr = httpd->begin(url, method, version, getargs, headers);

    *con_cls = c;
  }

  connection_t* c = (connection_t*)*con_cls;

  if(c == nullptr) return MHD_NO;

  if(*data_size && data) {
    if(!httpd->data(c->ptr, data, *data_size)) return MHD_NO;
  }

  if(c->total == c->acc && c->firstrun == false) {
    Httpd::Reply reply;
    if(!httpd->complete(c->ptr, reply)) return MHD_NO;

    struct MHD_Response *rsp;
    rsp = MHD_create_response_from_data(reply.data.length(),
                                        (void*)reply.data.data(),
                                        MHD_NO,   // must free
                                        MHD_YES); // must copy

    headers_t::iterator i = reply.headers.begin();
    while(i != reply.headers.end()) {
      MHD_add_response_header(rsp, i->first.c_str(), i->second.c_str());
      i++;
    }

    ret = MHD_queue_response(con, reply.status, rsp);


    MHD_destroy_response(rsp);
  }

  c->firstrun = false;
  c->acc += *data_size;

  *data_size = 0;
  return ret;
}

static void completed_handler(void *cls,
                              struct MHD_Connection *con,
                              void **con_cls,
                              enum MHD_RequestTerminationCode toe)
{
  Httpd *httpd = (Httpd*)cls;

  connection_t* c = (connection_t*)*con_cls;

  if(c) {
    httpd->cleanup(c->ptr);
    c->ptr = nullptr;
    delete c;
  }

  *con_cls = nullptr;
}

static void error_handler(void *cls, const char *fmt, va_list ap)
{
  Httpd *httpd = (Httpd*)cls;
  char *cmsg;
  int sz = vasprintf(&cmsg, fmt, ap);
  std::string msg;
  msg.append(cmsg, sz);
  httpd->error(msg);
  free(cmsg);
}

Httpd::Httpd()
{
  d = nullptr;
  d_ssl = nullptr;
}

Httpd::~Httpd()
{
  stop();
  stop_ssl();
}

void Httpd::listen(unsigned short int port,
                   unsigned int cn_limit, unsigned int cn_timeout)
{
  int flags = MHD_USE_DEBUG | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD;

  d = MHD_start_daemon(flags, port, nullptr, nullptr,
                       request_handler, this,
                       MHD_OPTION_NOTIFY_COMPLETED, completed_handler, this,
                       MHD_OPTION_EXTERNAL_LOGGER, error_handler, this,
                       MHD_OPTION_CONNECTION_LIMIT, cn_limit,
                       MHD_OPTION_CONNECTION_TIMEOUT, cn_timeout,
                       MHD_OPTION_END);

  if(!d) {
    //ERR(server, "Failed to initialise MHD_start_daemon!\n");
    return;
  }
}

void Httpd::listen_ssl(unsigned short int port,
                       std::string key, std::string cert,
                       unsigned int cn_limit, unsigned int cn_timeout)
{
  int flags = MHD_USE_DEBUG | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_SSL;

  d_ssl = MHD_start_daemon(flags, port, nullptr, nullptr,
                           request_handler, this,
                           MHD_OPTION_NOTIFY_COMPLETED, completed_handler, this,
                           MHD_OPTION_EXTERNAL_LOGGER, error_handler, this,
                           MHD_OPTION_CONNECTION_LIMIT, cn_limit,
                           MHD_OPTION_CONNECTION_TIMEOUT, cn_timeout,
                           MHD_OPTION_HTTPS_MEM_KEY, key.c_str(),
                           MHD_OPTION_HTTPS_MEM_CERT, cert.c_str(),
                           MHD_OPTION_END);

  if(!d_ssl) {
    //ERR(server, "Failed to initialise MHD_start_daemon!\n");
    return;
  }
}

void Httpd::stop()
{
  if(is_running()) {
    MHD_stop_daemon(d);
    d = nullptr;
  }
}

void Httpd::stop_ssl()
{
  if(is_running_ssl()) {
    MHD_stop_daemon(d_ssl);
    d_ssl = nullptr;
  }
}

bool Httpd::is_running()
{
  return d != nullptr;
}

bool Httpd::is_running_ssl()
{
  return d_ssl != nullptr;
}

#ifdef TEST_HTTPD
//deps: debug.cc log.cc mutex.cc
//cflags: -I.. $(HTTPD_CFLAGS) $(PTHREAD_CFLAGS)
//libs: $(HTTPD_LIBS) $(CURL_LIBS) $(PTHREAD_LIBS)
#include "test.h"

#include <curl/curl.h>

#define PORT 10008

#define LONG_LEN 20000

class TestHttpd : public Httpd {
public:
  TestHttpd() { fprintf(stderr, "TestHttpd()\n"); fflush(stderr); }
  ~TestHttpd() { fprintf(stderr, "~TestHttpd()\n"); fflush(stderr); }
  void error(const std::string &err)
  {
    fprintf(stderr, "ERROR: %s\n", err.c_str());
    fflush(stderr);
  }

  void *begin(const std::string &url,
              const std::string &method,
              const std::string &version,
              headers_t &headers)
  {
    fprintf(stderr, "begin(...)\n"); fflush(stderr);
    std::string *s = new std::string;

    headers_t::iterator i = headers.begin();
    while(i != headers.end()) {
      fprintf(stderr, "%s = \"%s\"\n", i->first.c_str(), i->second.c_str());
      fflush(stderr);
      i++;
    }
    /*
    if(headers.find("foo") != headers.end() && headers["foo"] == "bar")
      *s = "hdrok";
     */
    return s;
  }

  bool data(void *ptr, const char *data, unsigned int data_size)
  {
    std::string *s = (std::string*)ptr;
    if(data && data_size) s->append(data, data_size);

    fprintf(stderr, "data(...) (%p +%d %d)\n", ptr, data_size, s->length());
    fflush(stderr);

    return true;
  }

  bool complete(void *ptr, Httpd::Reply &reply)
  {
    std::string *s = (std::string*)ptr;
    fprintf(stderr, "complete(...) (%p %d)\n", ptr, s->length());
    fflush(stderr);
    reply.data = *s;
    reply.status = MHD_HTTP_OK;
    //reply.headers[MHD_HTTP_HEADER_CONTENT_TYPE] = "text/plain; charset=UTF-8";
    //reply.headers[MHD_HTTP_HEADER_CONTENT_TYPE] = "application/octet-stream";

    return true;
  }

  void cleanup(void *ptr)
  {
    fprintf(stderr, "cleanup(...)\n"); fflush(stderr);

    std::string *s = (std::string*)ptr;
    delete s;
  }
};

#if 0
static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  std::string *str = (std::string*)userp;
  str->append((char*)buffer, size * nmemb);
  return size * nmemb;
}

static std::string send(const std::string &msg, std::string name,
                        std::string value, CURLcode *ret)
{
  std::string response;

  CURL *c = curl_easy_init();
  curl_easy_setopt(c, CURLOPT_URL, "localhost");
  curl_easy_setopt(c, CURLOPT_PORT, PORT);
  curl_easy_setopt(c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(c, CURLOPT_TIMEOUT, 2L);
  curl_easy_setopt(c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 2L);
  curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(c, CURLOPT_USERAGENT, "TEST_HTTPD");

  curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, (long)msg.length());
  curl_easy_setopt(c, CURLOPT_POSTFIELDS, msg.data());
  curl_easy_setopt(c, CURLOPT_POST, 1L);

  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(c, CURLOPT_WRITEDATA, &response);

  struct curl_slist *slist=nullptr;
  slist = curl_slist_append(slist, (name + ": " + value).c_str());
  slist = curl_slist_append(slist, "Content-Type: application/octet-stream");

  curl_easy_setopt(c, CURLOPT_HTTPHEADER, slist);
    
  *ret = curl_easy_perform(c);
  
  curl_slist_free_all(slist);
  curl_easy_cleanup(c);

  return response;
}
#endif/*0*/

TEST_BEGIN;

TestHttpd httpd;
httpd.listen(PORT);
TEST_TRUE(httpd.is_running(), "Is the server running?");
/*
std::string r;

CURLcode errornum;
r = send("hello world", "foo", "bar", &errornum);
TEST_EQUAL_INT(errornum, CURLE_OK, "Did perfom go well?");
TEST_EQUAL_STR(r, "hello world", "Did we receive the correct answer?");

std::string msg;
msg.append(LONG_LEN, 0x00);
r = send(msg, "foo", "bar", &errornum);
TEST_EQUAL_INT(errornum, CURLE_OK, "Did perfom go well?");
TEST_EQUAL(r, msg, "Did we receive the correct answer?");
*/
TEST_END;

#endif/*TEST_HTTPD*/

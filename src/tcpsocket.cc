/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tcpsocket.cc
 *
 *  Thu Oct 19 10:24:25 CEST 2006
 *  Copyright  2006 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Artefact.
 *
 *  Artefact is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Artefact is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Artefact; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <config.h>

#include "tcpsocket.h"

#include <hugin.hpp>

//#define WITH_DEBUG

// for gethostbyname
#include <netdb.h>

// for inet_addr
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// for connect, listen, bind and accept
#include <sys/types.h>
#include <sys/socket.h>

// For socket
#include <sys/types.h>
#include <sys/socket.h>

// For TCP
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

// For inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// For close
#include <unistd.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>

// For ioctl
#include <sys/ioctl.h>

// For socket and friends
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

// For fcntl
#include <unistd.h>
#include <fcntl.h>

TCPSocket::TCPSocket(std::string name, int sock)
{
  this->name = name;
  if(sock == -1) {
    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
      throw TCPSocketException(strerror(errno));
    }
  }
  isconnected = false;
  this->sock = sock;

  DEBUG(socket, "TCPSocket %s: %p %d (%d)\n", name.c_str(), this, sock, getpid());
}

TCPSocket::~TCPSocket()
{
  DEBUG(socket, "~TCPSocket %s: %p %d (%d)\n",
        name.c_str(), this, sock, getpid());
  disconnect();
}


static int _listen(int sockfd, int backlog){return listen(sockfd, backlog);}
void TCPSocket::listen(unsigned short int port)
{

  if(sock == -1) throw TCPListenException("Socket not initialized.");
  if(isconnected) throw TCPListenException("Socket alread connected.");

  struct sockaddr_in socketaddr;
  memset((char *) &socketaddr, sizeof(socketaddr), 0);
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
  socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(sock, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) {
    throw TCPListenException(std::string("bind failed - ") + strerror(errno));
  }

  if(_listen(sock, 5) == -1) {
    throw TCPListenException(std::string("listen failed - ") + strerror(errno));
  }

  isconnected = true;
}

/**
 **
 ** Accept connections and block until one gets in.
 ** Return the new connection on incoming.
 ** It throws exceptions if an error occurres.
 ** On interrupts, it returns nullptr
 **
 **/
static int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{return accept(sockfd, addr, addrlen);}
TCPSocket *TCPSocket::accept()
{
  if(sock == -1) throw TCPAcceptException("Socket not initialized.");
  if(!isconnected) throw TCPAcceptException("Socket not connected.");

  // accept new connection and get its connection descriptor
  struct sockaddr_in ssocketaddr;
  int csalen = sizeof(ssocketaddr);

  // Select
  fd_set fset;
  int ret;
  FD_ZERO(&fset);
  FD_SET(sock, &fset);
  if( (ret = select (sock+1, &fset, nullptr, nullptr, nullptr)) < 0) { 
    if(errno == EINTR) {
      DEBUG(socket, "Accept got interrupt!\n");
      return nullptr; // a signal caused select to return. That is OK with me
    } else {
      throw TCPAcceptException("Select on socket failed.");
    }
  }
  if(FD_ISSET(sock, &fset)) {
    int csock = _accept(sock, (struct sockaddr*)&ssocketaddr, (socklen_t*)&csalen);
    TCPSocket *child = new TCPSocket(name + "-child", csock);
    
    if (child->sock == -1) {
      throw TCPAcceptException(std::string("accept failed - ") + strerror(errno));
    }
    child->isconnected = true;
    return child;
  } else {
    ERR(socket,
            "Accept returned with no socket - This should not happen!\n");
    return nullptr;
  }
}


static int _connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{return connect(sockfd, serv_addr, addrlen);}
void TCPSocket::connect(std::string addr, unsigned short int port)
{
  if(isconnected) throw TCPConnectException("Socket already connected.", "", "");

#ifndef BYPASS_STATICALLOCATIONS
  // Do DNS lookup
  char *ip;
  struct in_addr **addr_list;
  struct hostent *he;
  he = gethostbyname(addr.c_str());
  if(!he || !he->h_length) {
    char portno[32];
    sprintf(portno, "%d", port);
    throw TCPConnectException(addr, portno,
                              std::string("host lookup failed: ") + hstrerror(h_errno));
  }

  addr_list = (struct in_addr **)he->h_addr_list;
  //  Get first value. We know for sure that there are at least one.
  ip = inet_ntoa(*addr_list[0]);
#else/*BYPASS_STATICALLOCATIONS*/
  char *ip = "127.0.0.1";
#endif/*BYPASS_STATICALLOCATIONS*/

  struct sockaddr_in socketaddr;
  memset((char *) &socketaddr, sizeof(socketaddr), 0);
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
  socketaddr.sin_addr.s_addr = inet_addr(ip);

  if(_connect(sock, (struct sockaddr*)&socketaddr, sizeof(socketaddr))) {
    char portno[32];
    sprintf(portno, "%d", port);
    throw TCPConnectException(addr, portno, hstrerror(h_errno));
  }

  isconnected = true;
}

void TCPSocket::disconnect()
{
  if(sock != -1) {
    DEBUG(socket, "Closing TCPSocket %s: %p %d (%d)\n",
          name.c_str(), this, sock, getpid());
    int ret = close(sock);
    if(ret == -1) {
      perror(name.c_str());
    }
    sock = -1;
  }
  isconnected = false;
}

bool TCPSocket::connected()
{
  return sock != -1 && isconnected;
}



/**
 **
 ** Read read a number of bytes from the network.
 ** It returns the number of bytes read.
 ** It throws exceptions if an error occurres.
 ** On interrupts, it returns -1
 **
 **/
ssize_t _read(int fd, void *buf, size_t count) { return read(fd, buf, count); }
int TCPSocket::read(char *buf, int size, long timeout)
{
  int res = 0;

  if(sock == -1) throw TCPReadException("Socket not initialized.");
  if(!isconnected) throw TCPReadException("Socket is not connected.");

  // Select
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = timeout;

  struct timeval *ptv = nullptr;
  if(timeout >= 0) ptv = &tv;

  fd_set fset;
  int ret;
  FD_ZERO(&fset);
  FD_SET(sock, &fset);
  ret = select (sock+1, &fset, nullptr, nullptr, ptv);
  switch(ret) {
  case -1:
    if(errno == EINTR) {
      DEBUG(socket, "EINTR - got interrupt\n");
      return -1; // a signal caused select to return. That is OK with me
    } else {
      throw TCPReadException("Select on socket (read) failed.");
    }
    break;

  case 0:
    // timeout
    DEBUG(socket, "Timeout\n");
    break;

  default:
    if(FD_ISSET(sock, &fset)) {
      //      res = recv(sock, buf, size, MSG_DONTWAIT);
      if( (res = _read(sock, buf, size)) == -1 ) {
        throw TCPReadException(strerror(errno));
      }
    } else {
      DEBUG(socket, "FD_ISSET failed (timeout?)\n");
      return 0;
    }
  }

  return res;
}

ssize_t _write(int fd, const void *buf, size_t count) { return write(fd, buf, count); } 
int TCPSocket::write(char *data, int size)
{
  if(sock == -1) {
    throw TCPWriteException("Socket not initialized.");
  }

  if(!isconnected) {
    throw TCPWriteException("Socket is not connected.");
  }

  int res;
  //  if( (res = send(sock, data, size, MSG_WAITALL)) == -1 ) {
  if( (res = _write(sock, data, size)) == -1 ) {
    throw TCPWriteException(strerror(errno));
  }

  return res;
}

int TCPSocket::write(std::string data)
{
  return write((char*)data.c_str(), data.length());
}

std::string TCPSocket::srcaddr()
{
  std::string addr;

#ifndef BYPASS_STATICALLOCATIONS
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);
  if(getpeername(sock, (sockaddr*)&name, &namelen) == -1) {
    throw TCPNameException(strerror(errno));
  }

  addr = inet_ntoa(name.sin_addr);
#else/*BYPASS_STATICALLOCATIONS*/
  addr = "127.0.0.1";
#endif/*BYPASS_STATICALLOCATIONS*/

  return addr;
}

std::string TCPSocket::dstaddr()
{
  std::string addr;

#ifndef BYPASS_STATICALLOCATIONS
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);
  if(getsockname(sock, (sockaddr*)&name, &namelen) == -1) {
    throw TCPNameException(strerror(errno));
  }
  
  addr = inet_ntoa(name.sin_addr);
#else/*BYPASS_STATICALLOCATIONS*/
  addr = "127.0.0.1";
#endif/*BYPASS_STATICALLOCATIONS*/

  return addr;
}

#ifdef TEST_TCPSOCKET
//deps: exception.cc debug.cc log.cc mutex.cc
//cflags: -I.. $(PTHREAD_CFLAGS)
//libs: $(PTHREAD_LIBS)
#include <test.h>

#define PORT 12346

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).
TEST_TRUE(false, "No tests yet!");

/*
  char buf[32];

  switch(fork()) {
  case -1: // error
    fprintf(stderr, "Could not fork: %s\n", strerror(errno));
    return 1;
        
  case 0: // child
    try {
      TCPSocket client;
      sleep(1); // We need to wait for the listen socket to be created.
      client.connect("localhost", PORT);
      sprintf(buf, "hello");
      client.write(buf, sizeof(buf));
      printf("Sent: [%s]\n", buf);
    } catch( Exception &e ) {
      fprintf(stderr, "%s\n", e.what());
      return 1;
    }
    break;
        
  default: // parent
    try {
      TCPSocket listen_sock;
      listen_sock.listen(PORT);
      TCPSocket *sock = listen_sock.accept();
      sock->read(buf, sizeof(buf));
      printf("Got: [%s]\n", buf);
      delete sock;
      if(std::string(buf) != "hello") return 1;
    } catch( Exception &e ) {
      fprintf(stderr, "%s\n", e.what());
      return 1;
    }
    break;
  }
*/

TEST_END;

#endif/*TEST_TCPSOCKET*/

/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tcpsocket.h
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
#ifndef __ARTEFACT_TCPSOCKET_H__
#define __ARTEFACT_TCPSOCKET_H__

#include <string>

#include "exception.h"

/**
 * This exception is thrown by TCPSocket when the socket creation fails.
 */
class TCPSocketException: public Exception {
public:
  TCPSocketException(std::string reason) : 
    Exception("Could not create socket: " + reason) {}
};

/**
 * This exception is thrown by TCPSocket when listen fails.
 */
class TCPListenException: public Exception {
public:
  TCPListenException(std::string reason) : 
    Exception("Listen failed: " + reason) {}
};

/**
 * This exception is thrown by TCPSocket when accept fails.
 */
class TCPAcceptException: public Exception {
public:
  TCPAcceptException(std::string reason) : 
    Exception("Accept failed: " + reason) {}
};

/**
 * This exception is thrown by TCPSocket when connection fails.
 */
class TCPConnectException: public Exception {
public:
  TCPConnectException(std::string host, std::string port, std::string reason) : 
    Exception("Could not connect to " + host + ":" + port + ": " + reason) {}
};

/**
 * This exception is thrown by TCPSocket when reading fails.
 */
class TCPReadException: public Exception {
public:
  TCPReadException(std::string reason) : 
    Exception("TCPSocket could not read data: " + reason) {}
};

/**
 * This exception is thrown by TCPSocket when writing fails.
 */
class TCPWriteException: public Exception {
public:
  TCPWriteException(std::string reason) : 
    Exception("TCPSocket could not write data: " + reason) {}
};

/**
 * This exception is thrown by TCPSocket when there is an error in name lookup.
 */
class TCPNameException: public Exception {
public:
  TCPNameException(std::string reason) : 
    Exception("TCPSocket could not get name: " + reason) {}
};

/**
 * This class is used to commumicate through a TCP/IP connection, wether it
 * is a server (listening) or a client (transmitting).
 */
class TCPSocket {
public:
  /**
   * Constructor. Creates a new tcp socket.
   */
  TCPSocket(std::string name = "", int sock = -1);

  /**
   * Destructor. Closes the tcp socket.
   */
  ~TCPSocket();
    
  /**
   * Sets the socket in listen mode.\n
   * @param port The port number on which to listen.
   */
  void listen(unsigned short int port);
    
  /**
   * Accept an incoming connection.\n
   * The call is blocking and returns only when an incoming connection is received.\n
   * The socket must be in listen mode in order for this call to work.\n
   * Multiple accepts can be made on the same listening socket.
   * @return A connected TCPSocket ready to communicate.
   */
  TCPSocket *accept();
    
  /**
   * Connects to a host for data transmission.
   * @param addr The address of the host to connect to.
   * @param port The portnumber of the host to connect to.
   */
  void connect(std::string addr, unsigned short int port);
    
  /**
   * Disconnect the socket.
   */
  void disconnect();
    
  /**
   * Tells whether the socket is connected or not.
   * @return true if the socket is connected, false if not.
   */ 
  bool connected();
    
  /**
   * Reads bytes from the socket into a buffer.
   * @param buf The buffer into which the data will be written.
   * @param size The maximum number of bytes to read in (the size of the buffer).
   * @parasm timeout The timeout in ms, -1 is no timeout. -1 is default.
   * @return The actual number of bytes read.
   */
  int read(char *buf, int size, long timeout = -1);
    
  /**
   * Writes bytes from a buffer to the socket.
   * @param data The buffer from which the data will be read.
   * @param size The number of bytes to write.
   * @return The actual number of bytes written.
   */
  int write(char *data, int size);
  int write(std::string data);
    
  /**
   * Get the source address of the socket (IP address not DNS name).
   * @return An STL string containing the source address.
   */
  std::string srcaddr();

  /**
   * Get the destination address of the socket (IP address not DNS name).
   * @return An STL string containing the destination address.
   */
  std::string dstaddr();

private:
  bool isconnected;
  int sock;
  std::string name;
};


#endif/*__ARTEFACT_TCPSOCKET_H__*/

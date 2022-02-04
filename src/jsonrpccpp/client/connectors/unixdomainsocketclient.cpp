/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    unixdomainsocketclient.cpp
 * @date    11.05.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "unixdomainsocketclient.h"
#include "../../common/sharedconstants.h"
#include "../../common/streamreader.h"
#include "../../common/streamwriter.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace jsonrpc;
using namespace std;

UnixDomainSocketClient::UnixDomainSocketClient(const std::string &path)
    : path(path) {}

UnixDomainSocketClient::~UnixDomainSocketClient() {}

void UnixDomainSocketClient::SendRPCMessage(const std::string &message,
                                            std::string &result,
                                            stringstream &oss) {
  sockaddr_un address;
  int socket_fd;
  oss << "HI4 ";

  socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    oss << "HIE4 ";
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR,
                           "Could not create unix domain socket");
  }
  memset(&address, 0, sizeof(sockaddr_un));

  address.sun_family = AF_UNIX;
  strncpy(address.sun_path, this->path.c_str(), 107);

  oss << "HI5 ";
  if (connect(socket_fd, (struct sockaddr *)&address, sizeof(sockaddr_un)) !=
      0) {
    close(socket_fd);
    oss << "HIE5 ";
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR,
                           "Could not connect to: " + this->path);
  }

  StreamWriter writer;
  string toSend = message + DEFAULT_DELIMITER_CHAR;
  oss << "HI6 ";
  if (!writer.Write(toSend, socket_fd)) {
    close(socket_fd);
    oss << "HIE6 ";
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR,
                           "Could not write request");
  }

  StreamReader reader(DEFAULT_BUFFER_SIZE);
  oss << "HI7 ";
  if (!reader.Read(result, socket_fd, DEFAULT_DELIMITER_CHAR)) {
    close(socket_fd);
    oss << "HIE7 ";
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR,
                           "Could not read response");
  }
  oss << "HI8 ";
  close(socket_fd);
}

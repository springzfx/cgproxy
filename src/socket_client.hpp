#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include "common.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

namespace CGPROXY::SOCKET {

#define return_if_error(flag, msg)                                                       \
  if (flag == -1) {                                                                      \
    perror(msg);                                                                         \
    status = CONN_ERROR;                                                                 \
    close(sfd);                                                                          \
    return;                                                                              \
  }

void send(const char *msg, int &status);
void send(const string msg, int &status);

} // namespace CGPROXY::SOCKET
#endif
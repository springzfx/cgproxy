#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include "common.hpp"
#include <filesystem>
#include <functional>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
using namespace std;
namespace fs = std::filesystem;

namespace CGPROXY::SOCKET {

#define continue_if_error(flag, msg)                                                     \
  if (flag == -1) {                                                                      \
    perror(msg);                                                                         \
    continue;                                                                            \
  }

struct thread_arg {
  function<int(char *)> handle_msg;
};

class SocketServer {
public:
  int sfd = -1, cfd = -1, flag = -1;
  struct sockaddr_un unix_socket;

  void socketListening(function<int(char *)> callback);
  ~SocketServer();
  static void *startThread(void *arg);
};

} // namespace CGPROXY::SOCKET

#endif
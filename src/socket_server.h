#pragma once

#include <functional>
#include <future>
#include <stdlib.h>
#include <sys/un.h>

namespace CGPROXY::SOCKET {

#define continue_if_error(flag, msg)                                                     \
  if (flag == -1) {                                                                      \
    perror(msg);                                                                         \
    continue;                                                                            \
  }

class SocketServer {
public:
  int sfd = -1, cfd = -1, flag = -1;
  struct sockaddr_un unix_socket;

  void socketListening(const std::function<int(char *)> &callback, std::promise<void> status);
  ~SocketServer();
};

void startThread(const std::function<int(char *)> &callback, std::promise<void> status);

} // namespace CGPROXY::SOCKET

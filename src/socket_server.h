#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <functional>
#include <future>
#include <stdlib.h>
#include <sys/un.h>
using namespace std;

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

  void socketListening(function<int(char *)> callback, promise<void> status);
  ~SocketServer();
};

void startThread(function<int(char *)> callback, promise<void> status);

} // namespace CGPROXY::SOCKET

#endif
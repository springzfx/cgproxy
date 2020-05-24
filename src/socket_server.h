#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <functional>
#include <stdlib.h>
#include <sys/un.h>
using namespace std;

namespace CGPROXY::SOCKET {

#define continue_if_error(flag, msg)                                                     \
  if (flag == -1) {                                                                      \
    perror(msg);                                                                         \
    continue;                                                                            \
  }

struct thread_arg {
  function<int(char *)> handle_msg;
};
void *startThread(void *arg);

class SocketServer {
public:
  int sfd = -1, cfd = -1, flag = -1;
  struct sockaddr_un unix_socket;

  void socketListening(function<int(char *)> callback);
  ~SocketServer();
};

} // namespace CGPROXY::SOCKET

#endif
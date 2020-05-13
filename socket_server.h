#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include "common.h"
#include <functional>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;

#define SOCKET_PATH "/tmp/unix_socket"
#define LISTEN_BACKLOG 5

class SocketControl;
struct thread_arg;

#define continue_if_error(flag, msg)                                           \
  if (flag == -1) {                                                            \
    perror(msg);                                                               \
    continue;                                                                  \
  }

struct thread_arg {
  SocketControl *sc;
  function<int(char *)> handle_msg;
};

class SocketControl {
public:
  int sfd = -1, cfd = -1, flag = -1;
  struct sockaddr_un unix_socket;

  void socketListening(function<int(char *)> callback) {
    debug("starting socket listening");
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);

    unlink(SOCKET_PATH);
    memset(&unix_socket, '\0', sizeof(struct sockaddr_un));
    unix_socket.sun_family = AF_UNIX;
    strncpy(unix_socket.sun_path, SOCKET_PATH,
            sizeof(unix_socket.sun_path) - 1);

    bind(sfd, (struct sockaddr *)&unix_socket, sizeof(struct sockaddr_un));

    listen(sfd, LISTEN_BACKLOG);
    chmod(SOCKET_PATH,S_IRWXU|S_IRWXG|S_IRWXO);

    while (true) {
      close(cfd);
      cfd = accept(sfd, NULL, NULL);
      continue_if_error(cfd, "accept");
      debug("accept connection: %d", cfd);

      // read length
      int msg_len;
      flag = read(cfd, &msg_len, sizeof(int));
      continue_if_error(flag, "read length");
      // read msg
      char msg[msg_len];
      flag = read(cfd, msg, msg_len * sizeof(char));
      continue_if_error(flag, "read msg");
      msg[msg_len]='\0';
      // handle msg
      int status = callback(msg);
      // send back flag
      flag = write(cfd, &status, sizeof(int));
      continue_if_error(flag, "write back");
    }
  }

  ~SocketControl() {
    close(sfd);
    close(cfd);
    unlink(SOCKET_PATH);
  }

  static void *startThread(void *arg) {
    thread_arg *p = (thread_arg *)arg;
    p->sc->socketListening(p->handle_msg);
  }
};

#endif
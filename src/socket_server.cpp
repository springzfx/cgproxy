#include "socket_server.h"
#include "common.h"
#include <filesystem>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace CGPROXY::SOCKET {

void SocketServer::socketListening(function<int(char *)> callback, promise<void> status) {
  debug("starting socket listening");
  sfd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (fs::exists(SOCKET_PATH) && unlink(SOCKET_PATH) == -1) {
    error("%s exist, and can't unlink", SOCKET_PATH);
    return;
  }
  memset(&unix_socket, '\0', sizeof(struct sockaddr_un));
  unix_socket.sun_family = AF_UNIX;
  strncpy(unix_socket.sun_path, SOCKET_PATH, sizeof(unix_socket.sun_path) - 1);

  bind(sfd, (struct sockaddr *)&unix_socket, sizeof(struct sockaddr_un));

  listen(sfd, LISTEN_BACKLOG);
  chmod(SOCKET_PATH, S_IRWXU | S_IRWXG | S_IRWXO);

  status.set_value();

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
    auto msg = (char *)malloc(msg_len + 1);
    flag = read(cfd, msg, msg_len * sizeof(char));
    continue_if_error(flag, "read msg");
    msg[msg_len] = '\0';
    // handle msg
    int status = callback(msg);
    free(msg);
    // send back flag
    flag = write(cfd, &status, sizeof(int));
    continue_if_error(flag, "write back");
  }
}

SocketServer::~SocketServer() {
  close(sfd);
  close(cfd);
  unlink(SOCKET_PATH);
}

void startThread(function<int(char *)> callback, promise<void> status) {
  SocketServer server;
  server.socketListening(callback, move(status));
}

} // namespace CGPROXY::SOCKET
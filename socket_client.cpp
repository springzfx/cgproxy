#include "common.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;

#define return_if_error(flag, msg)                                             \
  if (flag == -1) {                                                            \
    perror(msg);                                                               \
    status = CONN_ERROR;                                                       \
    return;                                                                    \
  }

void send(char *msg, int &status) {
  debug("send msg: %s", msg);
  status = UNKNOWN_ERROR;

  int flag;
  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un unix_socket;
  memset(&unix_socket, '\0', sizeof(struct sockaddr_un));
  unix_socket.sun_family = AF_UNIX;
  strncpy(unix_socket.sun_path, SOCKET_PATH, sizeof(unix_socket.sun_path) - 1);

  flag =
      connect(sfd, (struct sockaddr *)&unix_socket, sizeof(struct sockaddr_un));
  return_if_error(flag, "connect");

  int msg_len = strlen(msg);
  flag = write(sfd, &msg_len, sizeof(int));
  return_if_error(flag, "write length");
  flag = write(sfd, msg, msg_len * sizeof(char));
  return_if_error(flag, "write msg");

  flag = read(sfd, &status, sizeof(int));
  return_if_error(flag, "read return value");

  close(sfd);
}

void send(const json &j, int &status) {
  string msg = j.dump();
  int msg_len = msg.length();
  char buff[msg_len];
  msg.copy(buff, msg_len, 0);
  buff[msg_len] = '\0';
  send(buff, status);
  debug("return status: %d", status);
}

int test_json() {
  json j;
  j["type"] = MSG_TYPE_JSON;
  j["data"]["cgroup_proxy"] = "/";
  j["data"]["enable_dns"] = false;
  int status;
  send(j, status);
}

void test_file() {
  json j;
  j["type"] = MSG_TYPE_CONFIG_PATH;
  j["data"] = "/etc/cgproxy.conf";
  int status;
  send(j, status);
}

int main() {
  test_file();
  test_json();
}
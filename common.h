#ifndef COMMON_H
#define COMMON_H

#define SOCKET_PATH "/tmp/unix_socket"
#define LISTEN_BACKLOG 5
#define DEFAULT_CONFIG_FILE "/etc/cgproxy.conf"

#define MSG_TYPE_JSON 1
#define MSG_TYPE_CONFIG_PATH 2
#define MSG_TYPE_PROXY_PID 3
#define MSG_TYPE_NOPROXY_PID 4

#define UNKNOWN_ERROR -99
#define CONN_ERROR -1
#define MSG_ERROR 1
#define PARSE_ERROR 2
#define PARAM_ERROR 3
#define APPLY_ERROR 4

#include <iostream>
#include <sstream>
#include <string.h>
using namespace std;
template <typename... T> string to_str(T... args) {
  stringstream ss;
  ss.clear();
  (ss << ... << args) << endl;
  return ss.str();
}

#define error(...) {fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}
#define debug(...) {fprintf(stdout, __VA_ARGS__);fprintf(stdout, "\n");}

#endif
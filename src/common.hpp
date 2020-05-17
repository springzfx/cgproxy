#ifndef COMMON_H
#define COMMON_H 1

#define TPROXY_IPTABLS_START "sh /usr/share/cgproxy/scripts/cgroup-tproxy.sh"
#define TPROXY_IPTABLS_CLEAN "sh /usr/share/cgproxy/scripts/cgroup-tproxy.sh stop"

#define PID_LOCK_FILE "/var/run/cgproxyd.pid"
#define SOCKET_PATH "/tmp/cgproxy_unix_socket"
#define LISTEN_BACKLOG 64
#define DEFAULT_CONFIG_FILE "/etc/cgproxy/config.json"

#define CGROUP_PROXY_PRESVERED "/proxy.slice"
#define CGROUP_NOPROXY_PRESVERED "/noproxy.slice"

#define MSG_TYPE_JSON 1
#define MSG_TYPE_CONFIG_PATH 2
#define MSG_TYPE_PROXY_PID 3
#define MSG_TYPE_NOPROXY_PID 4

#define UNKNOWN_ERROR 99
#define ERROR -1
#define SUCCESS 0
#define CONN_ERROR 1
#define MSG_ERROR 2
#define PARSE_ERROR 3
#define PARAM_ERROR 4
#define APPLY_ERROR 5
#define CGROUP_ERROR 6
#define FILE_ERROR 7

#include <iostream>
#include <regex>
#include <sstream>
#include <string>
using namespace std;

extern bool enable_debug;

#define error(...)                                                                       \
  {                                                                                      \
    fprintf(stderr, __VA_ARGS__);                                                        \
    fprintf(stderr, "\n");                                                               \
  }
#define debug(...)                                                                       \
  if (enable_debug) {                                                                    \
    fprintf(stdout, __VA_ARGS__);                                                        \
    fprintf(stdout, "\n");                                                               \
  }
#define return_error return -1;
#define return_success return 0;

template <typename... T> string to_str(T... args) {
  stringstream ss;
  ss.clear();
  ss << std::boolalpha;
  (ss << ... << args);
  return ss.str();
}

string join2str(const vector<string> t, const char delm = ' ');

string join2str(const int argc, char **argv, const char delm = ' ');

bool validCgroup(const string cgroup);

bool validCgroup(const vector<string> cgroup);

bool validPid(const string pid);

bool validPort(const int port);

#endif
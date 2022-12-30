#ifndef COMMON_H
#define COMMON_H 1

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define TPROXY_IPTABLS_START "@CMAKE_INSTALL_FULL_DATADIR@/cgproxy/scripts/cgroup-tproxy.sh"
#define TPROXY_IPTABLS_CLEAN "@CMAKE_INSTALL_FULL_DATADIR@/cgproxy/scripts/cgroup-tproxy.sh stop"

#define LIBEXECSNOOP_SO "@CMAKE_INSTALL_FULL_LIBDIR@/cgproxy/libexecsnoop.so"
#define CGROUP2_MOUNT_POINT "/var/run/cgproxy/cgroup2"
#define PID_LOCK_FILE "/var/run/cgproxyd.pid"
#define SOCKET_PATH "/tmp/cgproxy_unix_socket"
#define LISTEN_BACKLOG 64
#define DEFAULT_CONFIG_FILE "@CMAKE_INSTALL_FULL_SYSCONFDIR@/cgproxy/config.json"
#define READ_SIZE_MAX 128

#define CGROUP_PROXY_PRESVERED "/proxy.slice"
#define CGROUP_NOPROXY_PRESVERED "/noproxy.slice"

#define THREAD_TIMEOUT 5

#define MSG_TYPE_CONFIG_JSON 1
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

extern bool enable_debug;
extern bool enable_info;

#define error(...)                                                                       \
  {                                                                                      \
    fprintf(stderr, "error: ");                                                          \
    fprintf(stderr, __VA_ARGS__);                                                        \
    fprintf(stderr, "\n");                                                               \
    fflush(stderr);                                                                      \
  }

#define warning(...)                                                                       \
  {                                                                                      \
    fprintf(stderr, "warning: ");                                                          \
    fprintf(stderr, __VA_ARGS__);                                                        \
    fprintf(stderr, "\n");                                                               \
    fflush(stderr);                                                                      \
  }

#define debug(...)                                                                       \
  if (enable_debug) {                                                                    \
    fprintf(stdout, "debug: ");                                                          \
    fprintf(stdout, __VA_ARGS__);                                                        \
    fprintf(stdout, "\n");                                                               \
    fflush(stdout);                                                                      \
  }

#define info(...)                                                                        \
  if (enable_info) {                                                                     \
    fprintf(stdout, "info: ");                                                           \
    fprintf(stdout, __VA_ARGS__);                                                        \
    fprintf(stdout, "\n");                                                               \
    fflush(stdout);                                                                      \
  }

#define return_error return -1
#define return_success return 0

template <typename... T> string to_str(T... args) {
  stringstream ss;
  ss.clear();
  ss << std::boolalpha;
  (ss << ... << args);
  return ss.str();
}

string join2str(const vector<string> t, const char delm = ' ');
string join2str(const int argc, char **argv, const char delm = ' ');
bool startWith(string prefix);

bool validCgroup(const string cgroup);
bool validCgroup(const vector<string> cgroup);
bool validPid(const string pid);
bool validPort(const int port);

bool fileExist(const string &path);
bool dirExist(const string &path);
vector<int> bash_pidof(const string &path);
string bash_which(const string &name);
string bash_readlink(const string &path);
string getRealExistPath(const string &name);

/**
 * whether cg1 belongs to cg2
 */
bool belongToCgroup(string cg1, string cg2);
bool belongToCgroup(string cg1, vector<string> cg2);
string getCgroup(const pid_t &pid);
string getCgroup(const string &pid);

#endif

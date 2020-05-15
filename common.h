#ifndef COMMON_H
#define COMMON_H

#define SOCKET_PATH "/tmp/unix_socket"
#define LISTEN_BACKLOG 64
#define DEFAULT_CONFIG_FILE "/etc/cgproxy.conf"

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
#include <sstream>
#include <string>
#include <regex>
using namespace std;

#define error(...) {fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}
#define debug(...) {fprintf(stdout, __VA_ARGS__);fprintf(stdout, "\n");}
#define return_error return -1;
#define return_success return 0;


template <typename... T> 
string to_str(T... args) {
  stringstream ss;
  ss.clear();
  (ss << ... << args) << endl;
  return ss.str();
}

template <typename T>
string join2str(const T t){
    string s;
    string delm=" ", prefix="(", tail=")", wrap="\"";
    for (const auto &e : t) 
        e!=*(t.end()-1)?s+=wrap+e+wrap+delm:s+=wrap+e+wrap;
    return prefix+s+tail;
}

bool validCgroup(const string cgroup){
  return regex_match(cgroup, regex("^/[a-zA-Z0-9\\-_./@]*$"));
}

bool validCgroup(const vector<string> cgroup){
  for (auto &e:cgroup){
    if (!regex_match(e, regex("^/[a-zA-Z0-9\\-_./@]*$"))){
      return false;
    }
  }
  return true;
}

bool validPid(const string pid){
  return regex_match(pid, regex("^[0-9]+$"));
}

bool validPort(const string port){
  return regex_match(port, regex("^[0-9]+$"));
}

#endif
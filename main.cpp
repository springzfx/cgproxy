#include "common.h"
#include "socket_server.h"
#include <fstream>
#include <iostream>
#include <libconfig.h++>
#include <nlohmann/json.hpp>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
using json = nlohmann::json;

struct Config {
  string cgroup_proxy = "/proxy.slice";
  string cgroup_noproxy = "/noproxy.slice";
  bool enable_gateway = false;
  int port = 12345;
  bool enable_dns = true;
  bool enable_tcp = true;
  bool enable_udp = true;
  bool enable_ipv4 = true;
  bool enable_ipv6 = true;

  void toEnv() {
    #define env(v) setenv(#v, to_str(v).c_str(), 1)
    env(cgroup_proxy);
    env(cgroup_noproxy);
    env(enable_gateway);
    env(port);
    env(enable_dns);
    env(enable_tcp);
    env(enable_udp);
    env(enable_ipv4);
    env(enable_ipv6);
    #undef env
  }

  int safeLoadFromFile(const string path){
    Config tmp=*this;
    int flag=tmp.loadFromFile(path);
    if (flag!=0) return flag;
    if (tmp.isValid()){
      loadFromFile(path);
      return 0;
    }else{
      return PARAM_ERROR;
    }
  }

  int safeLoadFromJson(const json& j){
    Config tmp=*this;
    int flag=tmp.loadFromJson(j);
    if (flag!=0) return flag;
    if (tmp.isValid()){
      loadFromJson(j);
      return 0;
    }else{
      return PARAM_ERROR;
    }
  }
  
  private:
  int loadFromFile(const string f) {
    debug("loading config: %s", f.c_str());
    libconfig::Config config_f;
    try { config_f.readFile(f.c_str()); } catch (exception &e) { return PARSE_ERROR; }
    #define assign(v, t) if (config_f.exists(#v)) {v = (t)config_f.lookup(#v);}
    assign(cgroup_proxy, string);
    assign(cgroup_noproxy, string);
    assign(enable_gateway, bool);
    assign(port, int);
    assign(enable_dns, bool);
    assign(enable_tcp, bool);
    assign(enable_udp, bool);
    assign(enable_ipv4, bool);
    assign(enable_ipv6, bool);
    #undef assign
    return 0;
  }

  int loadFromJson(const json &j) {
    #define get_to(v) try {j.at(#v).get_to(v); } catch (exception& e) {}
    get_to(cgroup_proxy);
    get_to(cgroup_noproxy);
    get_to(enable_gateway);
    get_to(port);
    get_to(enable_dns);
    get_to(enable_tcp);
    get_to(enable_udp);
    get_to(enable_ipv4);
    get_to(enable_ipv6);
    #undef get_to
    return 0;
  }

  bool isValid(){
    // TODO
    return true;
  }
};

SocketControl sc;
thread_arg arg_t;
Config config_tproxy;
pthread_t socket_thread_id = -1;

int applyConfig(Config *c) {
  system("sh /usr/share/cgproxy/scripts/cgroup-tproxy.sh stop");
  c->toEnv();
  system("sh /usr/share/cgproxy/scripts/cgroup-tproxy.sh");
  return 0;
}

int handle_msg(char *msg) {
  debug("received msg: %s", msg);
  json j;
  try{ j = json::parse(msg); }catch(exception& e){debug("msg paser error");return MSG_ERROR;}
  int type = -1, status;
  try {
    type = j.at("type").get<int>();
    if (type == MSG_TYPE_JSON) { // json data
      status=config_tproxy.safeLoadFromJson(j.at("data"));
    } else if (type == MSG_TYPE_CONFIG_PATH) { // config file
      status=config_tproxy.safeLoadFromFile(j.at("data").get<string>());
    }
  } catch (out_of_range &e) {
    return MSG_ERROR;
  }
  if (status==0){
    return applyConfig(&config_tproxy);
  }
  return status;
}

pthread_t startSocketListeningThread() {
  arg_t.sc = &sc;
  arg_t.handle_msg = &handle_msg;
  pthread_t thread_id;
  int status =
      pthread_create(&thread_id, NULL, &SocketControl::startThread, &arg_t);
  if (status != 0)
    error("socket thread create failed");
  return thread_id;
}

int main() {
  bool enable_socket = true;
  string config_path = DEFAULT_CONFIG_FILE;
  config_tproxy.safeLoadFromFile(config_path);
  applyConfig(&config_tproxy);
  if (enable_socket) {
    socket_thread_id = startSocketListeningThread();
    pthread_join(socket_thread_id, NULL);
  }
  return 0;
}

// TODO handle attch pid
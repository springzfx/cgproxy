#include "common.hpp"
#include "socket_server.hpp"
#include <fstream>
#include <iostream>
#include <libconfig.h++>
#include <nlohmann/json.hpp>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "config.hpp"
#include "cgroup_attach.hpp"

using namespace std;
using json = nlohmann::json;
using namespace CGPROXY::SOCKET;
using namespace CGPROXY::CONFIG;
using namespace CGPROXY::CGROUP;

SocketServer sc;
thread_arg arg_t;
Config config;
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

  int type, status;
  int pid, cgroup_target;
  try {
    type = j.at("type").get<int>();
    switch (type)
    {
    case MSG_TYPE_JSON:
      status=config.loadFromJson(j.at("data"));
      if (status==SUCCESS) status=applyConfig(&config);
      return status;
      break;
    case MSG_TYPE_CONFIG_PATH:
      status=config.loadFromFile(j.at("data").get<string>());
      if (status==SUCCESS) status=applyConfig(&config);
      return status;
      break;
    case MSG_TYPE_PROXY_PID:
      pid=j.at("data").get<int>();
      status=attach(pid, config.cgroup_proxy_preserved);
      return status;
      break;
    case MSG_TYPE_NOPROXY_PID:
      pid=j.at("data").get<int>();
      status=attach(pid, config.cgroup_noproxy_preserved);
      return status;
      break;
    default:
        return MSG_ERROR;
      break;
    };
  } catch (out_of_range &e) {
    return MSG_ERROR;
  } catch (exception &e){
    return ERROR;
  }
}

pthread_t startSocketListeningThread() {
  arg_t.sc = &sc;
  arg_t.handle_msg = &handle_msg;
  pthread_t thread_id;
  int status =
      pthread_create(&thread_id, NULL, &SocketServer::startThread, &arg_t);
  if (status != 0)
    error("socket thread create failed");
  return thread_id;
}

int main(int argc, char* argv[]) {
  int shift=1;
  processArgs(argc,argv,shift);

  bool enable_socket = true;
  string config_path = DEFAULT_CONFIG_FILE;
  config.loadFromFile(config_path);
  applyConfig(&config);
  if (enable_socket) {
    socket_thread_id = startSocketListeningThread();
    pthread_join(socket_thread_id, NULL);
  }
  return 0;
}
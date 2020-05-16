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
#include <csignal>
#include "config.hpp"
#include "cgroup_attach.hpp"

using namespace std;
using json = nlohmann::json;
using namespace CGPROXY::SOCKET;
using namespace CGPROXY::CONFIG;
using namespace CGPROXY::CGROUP;

namespace CGPROXY{

class cgproxyd{
  thread_arg arg_t;
  Config config;
  pthread_t socket_thread_id = -1;

  static cgproxyd* instance;
  static int handle_msg_static(char* msg){
    if (!instance) {
      error("no cgproxyd instance assigned");
      return ERROR;
    }
    return instance->handle_msg(msg);
  }
  static void signalHandler( int signum ){
    debug("Signal %d received.", &signum);
    if (!instance){ error("no cgproxyd instance assigned");}
    else { instance->stop(); }
    exit(signum);
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
    arg_t.handle_msg = &handle_msg_static;
    pthread_t thread_id;
    int status =
        pthread_create(&thread_id, NULL, &SocketServer::startThread, &arg_t);
    if (status != 0)
      error("socket thread create failed");
    return thread_id;
  }

  void assignStaticInstance(){
    instance=this;
  }

  public:
  int start(int argc, char* argv[]) {
    signal(SIGINT, &signalHandler);
    signal(SIGTERM,&signalHandler);
    signal(SIGHUP,&signalHandler);

    int shift=1;
    processArgs(argc,argv,shift);

    config.loadFromFile(DEFAULT_CONFIG_FILE);
    applyConfig(&config);

    assignStaticInstance();
    socket_thread_id = startSocketListeningThread();
    pthread_join(socket_thread_id, NULL);
    return 0;
  }
  int applyConfig(Config *c) {
    system(TPROXY_IPTABLS_CLEAN);
    c->toEnv();
    system(TPROXY_IPTABLS_START);
    // no need to track running status
    return 0;
  }
  void stop(){
    debug("stopping");
    system(TPROXY_IPTABLS_CLEAN);
  }
  ~cgproxyd(){
    stop();
  }
};

cgproxyd* cgproxyd::instance=NULL;

}

int main(int argc, char* argv[]) {
  CGPROXY::cgproxyd d;
  return d.start(argc,argv);
}
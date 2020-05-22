#ifndef CGPROXYD_HPP
#define CGPROXYD_HPP

#include "cgroup_attach.h"
#include "common.h"
#include "config.h"
#include "socket_server.h"
#include <csignal>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sched.h>
#include <sys/file.h>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;
using namespace ::CGPROXY::SOCKET;
using namespace ::CGPROXY::CONFIG;
using namespace ::CGPROXY::CGROUP;

namespace CGPROXY::CGPROXYD {

bool print_help = false;
bool enable_execsnoop = false;

class cgproxyd {
  thread_arg arg_t;
  Config config;
  pthread_t socket_thread_id = -1;
  pid_t exec_snoop_pid = -1;

  static cgproxyd *instance;
  static int handle_msg_static(char *msg) {
    if (!instance) {
      error("no cgproxyd instance assigned");
      return ERROR;
    }
    return instance->handle_msg(msg);
  }

  static void signalHandler(int signum) {
    debug("Signal %d received.", signum);
    if (!instance) {
      error("no cgproxyd instance assigned");
    } else {
      instance->stop();
    }
    exit(0);
  }

  // single process instance
  int lock_fd;
  void lock() {
    lock_fd = open(PID_LOCK_FILE, O_CREAT | O_RDWR, 0666);
    int rc = flock(lock_fd, LOCK_EX | LOCK_NB);
    if (rc == -1) {
      perror(PID_LOCK_FILE);
      error("maybe another cgproxyd is running");
      exit(EXIT_FAILURE);
    } else {
      ofstream ofs(PID_LOCK_FILE);
      ofs << getpid() << endl;
      ofs.close();
    }
  }
  void unlock() {
    close(lock_fd);
    unlink(PID_LOCK_FILE);
  }

  int handle_msg(char *msg) {
    debug("received msg: %s", msg);
    json j;
    try {
      j = json::parse(msg);
    } catch (exception &e) {
      debug("msg paser error");
      return MSG_ERROR;
    }

    int type, status;
    int pid, cgroup_target;
    try {
      type = j.at("type").get<int>();
      switch (type) {
      case MSG_TYPE_CONFIG_JSON:
        status = config.loadFromJsonStr(j.at("data").dump());
        if (status == SUCCESS) status = applyConfig(&config);
        return status;
        break;
      case MSG_TYPE_CONFIG_PATH:
        status = config.loadFromFile(j.at("data").get<string>());
        if (status == SUCCESS) status = applyConfig(&config);
        return status;
        break;
      case MSG_TYPE_PROXY_PID:
        pid = j.at("data").get<int>();
        status = attach(pid, config.cgroup_proxy_preserved);
        return status;
        break;
      case MSG_TYPE_NOPROXY_PID:
        pid = j.at("data").get<int>();
        status = attach(pid, config.cgroup_noproxy_preserved);
        return status;
        break;
      default: return MSG_ERROR; break;
      };
    } catch (out_of_range &e) { return MSG_ERROR; } catch (exception &e) {
      return ERROR;
    }
  }

  pthread_t startSocketListeningThread() {
    arg_t.handle_msg = &handle_msg_static;
    pthread_t thread_id;
    int status = pthread_create(&thread_id, NULL, &SocketServer::startThread, &arg_t);
    if (status != 0) error("socket thread create failed");
    return thread_id;
  }

  void startExecSnoopProc() {
    if (exec_snoop_pid != -1){ 
      kill(exec_snoop_pid, SIGINT);
      exec_snoop_pid=-1;
    }
    pid_t pid = fork();
    if (pid == 0) {
      execl(BPF_EXEC_SNOOP_START, (char *) NULL);
      exit(0);
    } else if (pid<0){
      error("fork precess failed");
    }else {
      exec_snoop_pid = pid;
    }
  }

  void assignStaticInstance() { instance = this; }

public:
  int start() {
    lock();
    signal(SIGINT, &signalHandler);
    signal(SIGTERM, &signalHandler);
    signal(SIGHUP, &signalHandler);

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
    if (enable_execsnoop) startExecSnoopProc();
    // no need to track running status
    return 0;
  }

  void stop() {
    debug("stopping");
    system(TPROXY_IPTABLS_CLEAN);
    // if (exec_snoop_pid != -1) kill(exec_snoop_pid, SIGINT);
    unlock();
  }

  ~cgproxyd() { stop(); }
};

cgproxyd *cgproxyd::instance = NULL;

void print_usage() {
  cout << "Start a daemon with unix socket to accept control" << endl;
  cout << "Usage: cgproxyd [--help] [--debug]" << endl;
  cout << "Alias: cgproxyd = cgproxy --daemon" << endl;
}

void processArgs(const int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0) { enable_debug = true; }
    if (strcmp(argv[i], "--help") == 0) { print_help = true; }
    if (strcmp(argv[i], "--execsnoop") == 0) { enable_execsnoop = true; }
    if (argv[i][0] != '-') { break; }
  }
}

int main(int argc, char *argv[]) {
  processArgs(argc, argv);
  if (print_help) {
    print_usage();
    exit(0);
  }

  if (getuid() != 0) {
    error("permission denied, need root");
    exit(EXIT_FAILURE);
  }

  cgproxyd d;
  return d.start();
}
} // namespace CGPROXY::CGPROXYD
#endif
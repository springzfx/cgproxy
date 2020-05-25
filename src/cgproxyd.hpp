#ifndef CGPROXYD_HPP
#define CGPROXYD_HPP

#include "cgroup_attach.h"
#include "common.h"
#include "config.h"
#include "execsnoop.h"
#include "socket_server.h"
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <dlfcn.h>
#include <exception>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>
#include <pthread.h>
#include <sched.h>
#include <sys/file.h>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;
using namespace ::CGPROXY::SOCKET;
using namespace ::CGPROXY::CONFIG;
using namespace ::CGPROXY::CGROUP;
// using namespace ::CGPROXY::EXECSNOOP;

namespace CGPROXY::EXECSNOOP {
typedef void *(*startThread_t)(void *arg);
startThread_t _startThread;
bool loadExecsnoopLib() {
  try {
    info("loading %s", LIBEXECSNOOP_SO);
    void *handle_dl = dlopen(LIBEXECSNOOP_SO, RTLD_NOW);
    if (handle_dl == NULL) {
      error("dlopen %s failed: %s", LIBEXECSNOOP_SO, dlerror());
      return false;
    }
    _startThread = reinterpret_cast<startThread_t>(dlsym(handle_dl, "_startThread"));
    if (_startThread == NULL) {
      error("dlsym startThread failed: %s", dlerror());
      return false;
    }
    info("dlsym startThread success");
    return true;
  } catch (exception &e) { return false; }
}
} // namespace CGPROXY::EXECSNOOP

namespace CGPROXY::CGPROXYD {

bool print_help = false;
bool enable_socketserver = true;
bool enable_execsnoop = false;

class cgproxyd {
  SOCKET::thread_arg socketserver_thread_arg;
  pthread_t socket_thread_id = THREAD_UNDEF;

  EXECSNOOP::thread_arg execsnoop_thread_arg;
  pthread_t execsnoop_thread_id = THREAD_UNDEF;

  Config config;

  static cgproxyd *instance;
  static int handle_msg_static(char *msg) {
    if (!instance) {
      error("no cgproxyd instance assigned");
      return ERROR;
    }
    return instance->handle_msg(msg);
  }

  static int handle_pid_static(int pid) {
    if (!instance) {
      error("no cgproxyd instance assigned");
      return ERROR;
    }
    return instance->handle_pid(pid);
  }

  int handle_pid(int pid) {
    auto path = realpath(to_str("/proc/", pid, "/exe").c_str(), NULL);
    if (path == NULL) {
      debug("pid %d live life too short", pid);
      return 0;
    }
    debug("execsnoop: %d %s", pid, path);

    vector<string> v;

    v = config.program_noproxy;
    if (find(v.begin(), v.end(), path) != v.end()) {
      if (CGROUP::getCgroup(pid) != config.cgroup_noproxy_preserved) {
        info("execsnoop noproxy: %d %s", pid, path);
        free(path);
        return attach(pid, config.cgroup_noproxy_preserved);
      }
    }
    v = config.program_proxy;
    if (find(v.begin(), v.end(), path) != v.end()) {
      if (CGROUP::getCgroup(pid) != config.cgroup_proxy_preserved) {
        info("execsnoop proxied: %d %s", pid, path);
        free(path);
        return attach(pid, config.cgroup_proxy_preserved);
      }
    }
    free(path);
    return 0;
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
        if (status == SUCCESS) status = applyConfig();
        return status;
        break;
      case MSG_TYPE_CONFIG_PATH:
        status = config.loadFromFile(j.at("data").get<string>());
        if (status == SUCCESS) status = applyConfig();
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
    socketserver_thread_arg.handle_msg = &handle_msg_static;
    pthread_t thread_id;
    int status =
        pthread_create(&thread_id, NULL, &SOCKET::startThread, &socketserver_thread_arg);
    if (status != 0) {
      error("socket thread create failed");
      return THREAD_UNDEF;
    }
    return thread_id;
  }

  pthread_t startExecsnoopThread() {
    if (!EXECSNOOP::loadExecsnoopLib() || EXECSNOOP::_startThread == NULL) {
      error("execsnoop start failed, maybe bcc not installed");
      return THREAD_UNDEF;
    }

    execsnoop_thread_arg.handle_pid = &handle_pid_static;
    pthread_t thread_id;
    int status =
        pthread_create(&thread_id, NULL, EXECSNOOP::_startThread, &execsnoop_thread_arg);
    if (status != 0) {
      error("execsnoop thread create failed");
      return THREAD_UNDEF;
    }
    return thread_id;
  }

  void processRunningProgram() {
    debug("process running program");
    for (auto &path : config.program_noproxy)
      for (auto &pid : bash_pidof(path)) {
        if (CGROUP::getCgroup(pid) != config.cgroup_noproxy_preserved) {
          int status = attach(pid, config.cgroup_noproxy_preserved);
          if (status == 0) info("noproxy running process %d %s", pid, path.c_str());
        }
      }
    for (auto &path : config.program_proxy)
      for (auto &pid : bash_pidof(path)) {
        if (CGROUP::getCgroup(pid) != config.cgroup_proxy_preserved) {
          int status = attach(pid, config.cgroup_proxy_preserved);
          if (status == 0) info("proxied running process %d %s", pid, path.c_str());
        }
      }
  }

  void assignStaticInstance() { instance = this; }

public:
  int start() {
    lock();
    signal(SIGINT, &signalHandler);
    signal(SIGTERM, &signalHandler);
    signal(SIGHUP, &signalHandler);

    assignStaticInstance();

    config.loadFromFile(DEFAULT_CONFIG_FILE);
    applyConfig();
    processRunningProgram();

    if (enable_socketserver) {
      socket_thread_id = startSocketListeningThread();
      if (socket_thread_id > 0) info("socket server listening thread started");
    }
    if (enable_execsnoop) {
      execsnoop_thread_id = startExecsnoopThread();
      if (execsnoop_thread_id > 0) info("execsnoop thread started");
    }
    cout << flush;

    pthread_join(socket_thread_id, NULL);
    pthread_join(execsnoop_thread_id, NULL);
    return 0;
  }
  int applyConfig() {
    system(TPROXY_IPTABLS_CLEAN);
    config.print_summary();
    config.toEnv();
    system(TPROXY_IPTABLS_START);
    // no need to track running status
    return 0;
  }

  void stop() {
    debug("stopping");
    system(TPROXY_IPTABLS_CLEAN);
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
  setbuf(stdout, NULL);
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

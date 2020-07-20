#ifndef CGPROXYD_HPP
#define CGPROXYD_HPP

#include "cgroup_attach.h"
#include "common.h"
#include "config.h"
#include "execsnoop_share.h"
#include "socket_server.h"
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <dlfcn.h>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <nlohmann/json.hpp>
#include <sched.h>
#include <sys/file.h>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;
using namespace ::CGPROXY::SOCKET;
using namespace ::CGPROXY::CONFIG;
using namespace ::CGPROXY::CGROUP;
// using namespace ::CGPROXY::EXECSNOOP;

#ifdef BUIlD_EXECSNOOP_DL
namespace CGPROXY::EXECSNOOP {
bool loadExecsnoopLib() {
  try {
    info("loading %s", LIBEXECSNOOP_SO);
    void *handle_dl = dlopen(LIBEXECSNOOP_SO, RTLD_NOW);
    if (handle_dl == NULL) {
      error("dlopen %s failed: %s", LIBEXECSNOOP_SO, dlerror());
      return false;
    }
    _startThread = reinterpret_cast<startThread_t *>(dlsym(handle_dl, "startThread"));
    if (_startThread == NULL) {
      error("dlsym startThread func failed: %s", dlerror());
      return false;
    }
    info("dlsym startThread func success");
    return true;
  } catch (exception &e) {
    debug("exception: %s", e.what());
    return false;
  }
}
} // namespace CGPROXY::EXECSNOOP
#endif

namespace CGPROXY::CGPROXYD {

bool print_help = false;
bool enable_socketserver = true;
bool enable_execsnoop = false;

class cgproxyd {
  thread socketserver_thread;
  thread execsnoop_thread;

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
    unique_ptr<char[], decltype(&free)> path(
        realpath(to_str("/proc/", pid, "/exe").c_str(), NULL), &free);
    if (path == NULL) {
      debug("execsnoop: pid %d live life too short", pid);
      return 0;
    }
    debug("execsnoop: %d %s", pid, path.get());

    vector<string> v;

    v = config.program_noproxy;
    if (find(v.begin(), v.end(), path.get()) != v.end()) {
      string cg = getCgroup(pid);
      if (cg.empty()) {
        debug("execsnoop: cgroup get failed, ignore: %d %s", pid, path.get());
        return 0;
      }
      if (belongToCgroup(cg, config.cgroup_proxy_preserved) ||
          belongToCgroup(cg, config.cgroup_noproxy_preserved)) {
        info("execsnoop: already in preserverd cgroup, leave alone: %d %s", pid,
             path.get());
        return 0;
      }
      if (!belongToCgroup(cg, config.cgroup_noproxy)) {
        int res = attach(pid, config.cgroup_noproxy_preserved);
        if (res == 0) {
          info("execsnoop; noproxy: %d %s", pid, path.get());
        } else {
          info("execsnoop; noproxy failed: %d %s", pid, path.get());
        }
        return res;
      }
    }

    v = config.program_proxy;
    if (find(v.begin(), v.end(), path.get()) != v.end()) {
      string cg = getCgroup(pid);
      if (cg.empty()) {
        debug("execsnoop: cgroup get failed, ignore: %d %s", pid, path.get());
        return 0;
      }
      if (belongToCgroup(cg, config.cgroup_proxy_preserved) ||
          belongToCgroup(cg, config.cgroup_noproxy_preserved)) {
        info("execsnoop: already in preserverd cgroup, leave alone: %d %s", pid,
             path.get());
        return 0;
      }
      if (!belongToCgroup(cg, config.cgroup_proxy)) {
        int res = attach(pid, config.cgroup_proxy_preserved);
        if (res == 0) {
          info("execsnoop: proxied: %d %s", pid, path.get());
        } else {
          info("execsnoop: proxied failed: %d %s", pid, path.get());
        }
        return res;
      }
    }
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

    int type, status, pid;
    try {
      type = j.at("type").get<int>();
      switch (type) {
      case MSG_TYPE_CONFIG_JSON:
        status = config.loadFromJsonStr(j.at("data").dump());
        info("process received config json msg");
        if (status == SUCCESS) status = applyConfig();
        return status;
        break;
      case MSG_TYPE_CONFIG_PATH:
        status = config.loadFromFile(j.at("data").get<string>());
        info("process received config path msg");
        if (status == SUCCESS) status = applyConfig();
        return status;
        break;
      case MSG_TYPE_PROXY_PID:
        pid = j.at("data").get<int>();
        info("process proxy pid msg: %d", pid);
        status = attach(pid, config.cgroup_proxy_preserved);
        return status;
        break;
      case MSG_TYPE_NOPROXY_PID:
        pid = j.at("data").get<int>();
        info("process noproxy pid msg: %d", pid);
        status = attach(pid, config.cgroup_noproxy_preserved);
        return status;
        break;
      default:
        error("unknown msg");
        return MSG_ERROR;
        break;
      };
    } catch (out_of_range &e) { return MSG_ERROR; } catch (exception &e) {
      return ERROR;
    }
  }

  void startSocketListeningThread() {
    promise<void> status;
    future<void> status_f = status.get_future();
    thread th(SOCKET::startThread, handle_msg_static, move(status));
    socketserver_thread = move(th);

    future_status fstatus = status_f.wait_for(chrono::seconds(THREAD_TIMEOUT));
    if (fstatus == std::future_status::ready) {
      info("socketserver thread started");
    } else {
      error("socketserver thread timeout, maybe failed");
    }
  }

  void startExecsnoopThread() {
    #ifdef BUIlD_EXECSNOOP_DL
    if (!EXECSNOOP::loadExecsnoopLib() || EXECSNOOP::_startThread == NULL) {
      error("execsnoop not ready to start, maybe missing libbpf");
      return;
    }
    #endif

    promise<void> status;
    future<void> status_f = status.get_future();
    #ifdef BUIlD_EXECSNOOP_DL
    thread th(EXECSNOOP::_startThread, handle_pid_static, move(status));
    #else
    thread th(EXECSNOOP::startThread, handle_pid_static, move(status));
    #endif

    execsnoop_thread = move(th);

    future_status fstatus = status_f.wait_for(chrono::seconds(THREAD_TIMEOUT));
    if (fstatus == std::future_status::ready) {
      info("execsnoop thread started");
      processRunningProgram();
    } else {
      error("execsnoop thread timeout, maybe failed");
    }
  }

  void processRunningProgram() {
    debug("process running program");
    for (auto &path : config.program_noproxy)
      for (auto &pid : bash_pidof(path)) {
        string cg = getCgroup(pid);
        if (cg.empty()) {
          debug("cgroup get failed, ignore: %d %s", pid, path.c_str());
          continue;
        }
        if (belongToCgroup(cg, config.cgroup_proxy_preserved) ||
            belongToCgroup(cg, config.cgroup_noproxy_preserved)) {
          info("already in preserverd cgroup, leave alone: %d %s", pid, path.c_str());
          continue;
        }
        if (!belongToCgroup(cg, config.cgroup_noproxy)) {
          int status = attach(pid, config.cgroup_noproxy_preserved);
          if (status == 0) info("noproxy running process %d %s", pid, path.c_str());
        }
      }
    for (auto &path : config.program_proxy)
      for (auto &pid : bash_pidof(path)) {
        string cg = getCgroup(pid);
        if (cg.empty()) {
          debug("cgroup get failed, ignore: %d %s", pid, path.c_str());
          continue;
        }
        if (belongToCgroup(cg, config.cgroup_proxy_preserved) ||
            belongToCgroup(cg, config.cgroup_noproxy_preserved)) {
          info("already in preserverd cgroup, leave alone: %d %s", pid, path.c_str());
          continue;
        }
        if (!belongToCgroup(cg, config.cgroup_proxy)) {
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

    if (config.loadFromFile(DEFAULT_CONFIG_FILE)!=SUCCESS) {
      error("load config file failed");
      return -1;
    }
    applyConfig();

    if (enable_socketserver) startSocketListeningThread();
    if (enable_execsnoop) startExecsnoopThread();

    if (socketserver_thread.joinable()) socketserver_thread.join();
    if (execsnoop_thread.joinable()) execsnoop_thread.join();

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

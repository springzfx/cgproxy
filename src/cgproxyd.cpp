#include "cgroup_attach.h"
#include "common.h"
#include "config.h"
#include "socket_server.h"
#include <csignal>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sys/file.h>
#include <unistd.h>

using namespace std;
using json = nlohmann::json;
using namespace CGPROXY::SOCKET;
using namespace CGPROXY::CONFIG;
using namespace CGPROXY::CGROUP;

namespace CGPROXY {

class cgproxyd {
  thread_arg arg_t;
  Config config;
  pthread_t socket_thread_id = -1;

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
    exit(signum);
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

} // namespace CGPROXY

bool print_help = false;

void print_usage() { printf("cgproxyd [--help] [--debug]\n"); }

void processArgs(const int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0) { enable_debug = true; }
    if (strcmp(argv[i], "--help") == 0) { print_help = true; }
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
    print_usage();
    exit(EXIT_FAILURE);
  }

  CGPROXY::cgproxyd d;
  return d.start();
}
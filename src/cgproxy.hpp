#include "common.h"
#include "config.h"
#include "socket_client.h"
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <unistd.h>
using json = nlohmann::json;
using namespace CGPROXY;
using namespace CGPROXY::CONFIG;

namespace CGPROXY::CGPROXY {

bool print_help = false, proxy = true;
inline void print_usage() {
  if (proxy) {
    cout << "Run program with proxy" << endl;
    cout << "Usage: cgproxy [--help] [--debug] <CMD>" << endl;
  } else {
    cout << "Run program without proxy" << endl;
    cout << "Usage: cgpnoroxy [--help] [--debug] <CMD>" << endl;
    cout << "Alias: cgnoproxy = cgproxy --noproxy" << endl;
  }
}

void processArgs(const int argc, char *argv[], int &shift) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--noproxy") == 0) { proxy = false; }
    if (strcmp(argv[i], "--debug") == 0) { enable_debug = true; }
    if (strcmp(argv[i], "--help") == 0) { print_help = true; }
    if (argv[i][0] != '-') { break; }
    shift += 1;
  }
}

void send_pid(const pid_t pid, bool proxy, int &status) {
  json j;
  j["type"] = proxy ? MSG_TYPE_PROXY_PID : MSG_TYPE_NOPROXY_PID;
  j["data"] = pid;
  SOCKET::send(j.dump(), status);
}

int main(int argc, char *argv[]) {
  int shift = 1;
  processArgs(argc, argv, shift);

  if (print_help) {
    print_usage();
    exit(0);
  }

  if (argc == shift) {
    error("no program specified");
    exit(EXIT_FAILURE);
  }

  int status = -1;
  send_pid(getpid(), proxy, status);
  if (status != 0) {
    error("attach process failed");
    if (status==1) error("maybe cgproxy.service not running");
    exit(EXIT_FAILURE);
  }

  string s = join2str(argc - shift, argv + shift, ' ');
  return system(s.c_str());
}
} // namespace CGPROXY::CGPROXY
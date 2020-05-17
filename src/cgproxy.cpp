#include "common.hpp"
#include "socket_client.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace CGPROXY;

bool print_help = false, proxy = true;
void print_usage() {
  fprintf(stdout, "Usage: cgproxy [--help] [--debug] [--noproxy] <CMD>\n");
  fprintf(stdout, "Alias: cgnoproxy = cgproxy --noproxy\n");
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

bool attach2cgroup(pid_t pid, bool proxy) {
  json j;
  j["type"] = proxy ? MSG_TYPE_PROXY_PID : MSG_TYPE_NOPROXY_PID;
  j["data"] = pid;
  int status;
  SOCKET::send(j.dump(), status);
  return status == 0;
}

int main(int argc, char *argv[]) {
  int shift = 1;
  processArgs(argc, argv, shift);

  if (argc == shift || print_help) {
    print_usage();
    exit(0);
  }

  pid_t pid = getpid();
  if (!attach2cgroup(pid, proxy)) {
    error("attach process failed");
    exit(EXIT_FAILURE);
  }

  string s = join2str(argc - shift, argv + shift, ' ');
  return system(s.c_str());
}
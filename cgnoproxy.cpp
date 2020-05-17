#include "socket_client.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace CGPROXY;

bool attach2cgproxy() {
  pid_t pid = getpid();
  json j;
  j["type"] = MSG_TYPE_NOPROXY_PID;
  j["data"] = pid;
  int status;
  SOCKET::send(j.dump(), status);
  return status == 0;
}

int main(int argc, char *argv[]) {
  int shift = 1;
  if (argc == 1) {
    error("usage: cgnoproxy [--debug] <CMD>\nexample: cgnoproxy curl -I "
          "https://www.google.com");
    exit(EXIT_FAILURE);
  }
  processArgs(argc, argv, shift);

  if (!attach2cgproxy()) {
    error("attach process failed");
    exit(EXIT_FAILURE);
  }

  string s = join2str(argc - shift, argv + shift, ' ');
  return system(s.c_str());
}
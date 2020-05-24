#include "execsnoop.hpp"
#include "common.h"
using namespace std;
using namespace CGPROXY::EXESNOOP;

#define PATH_MAX_LEN 128

int handle_pid(int pid) {
  char path[PATH_MAX_LEN];
  auto size = readlink(to_str("/proc/", pid, "/exe").c_str(), path, PATH_MAX_LEN);
  if (size == -1) error("readlink: %s", to_str("/proc/", pid, "/exe").c_str());
  path[size] = '\0';
  info("%d %s", pid, path);
  return 0;
}

int main() {
  enable_debug = true;
  enable_info = true;
  callback = handle_pid;
  execsnoop();
  return 0;
}

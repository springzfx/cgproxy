#include "cgroup_attach.h"
#include "common.h"
#include <cstdlib>
#include <unistd.h>
using namespace std;

void print_usage() { fprintf(stdout, "usage: cgattach <pid> <cgroup>\n"); }

int main(int argc, char *argv[]) {
  int flag = setuid(0);
  if (flag != 0) {
    perror("cgattach need root");
    exit(EXIT_FAILURE);
  }

  if (argc != 3) {
    error("need exact 2 paramaters");
    print_usage();
    exit(EXIT_FAILURE);
  }

  string pid = string(argv[1]);
  string cgroup_target = string(argv[2]);

  if (validPid(pid) && validCgroup(cgroup_target)) {
    CGPROXY::CGROUP::attach(pid, cgroup_target);
  } else {
    error("param not valid");
    exit(EXIT_FAILURE);
  }
}

#include "cgroup_attach.hpp"
using namespace std;

void print_usage() { fprintf(stdout, "usage: cgattach <pid> <cgroup>\n"); }

int main(int argc, char *argv[]) {
  int flag=setuid(0);
  if (flag!=0) {
    perror("cgattach setuid");
    exit(EXIT_FAILURE);
  }

  if (argc != 3) {
    error("only need 2 paramaters");
    print_usage();
    exit(EXIT_FAILURE);
  }

  string pid = string(argv[1]);
  string cgroup_target = string(argv[2]);

  CGPROXY::CGROUP::attach(pid,cgroup_target);
}

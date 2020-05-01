#include <errno.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

void print_usage() { fprintf(stdout, "usage: cgattach <pid> <cgroup>\n"); }

bool exist(string path) {
  struct stat st;
  if (stat(path.c_str(), &st) != -1) {
    return S_ISDIR(st.st_mode);
  }
  return false;
}

bool validate(string pid, string cgroup) {
  bool pid_v = regex_match(pid, regex("^[0-9]+$"));
  bool cg_v = regex_match(cgroup, regex("^\\/[a-zA-Z0-9\\-_./@]*$"));
  if (pid_v && cg_v)
    return true;
 
  fprintf(stderr, "paramater validate error\n");
  print_usage();
  exit(EXIT_FAILURE);
}

string get_cgroup2_mount_point(){
  char cgroup2_mount_point[100]="";
  FILE* fp = popen("findmnt -t cgroup2 -n -o TARGET", "r");
  int count=fscanf(fp,"%s",&cgroup2_mount_point);
  fclose(fp);
  if (count=0){
    fprintf(stderr, "cgroup2 not supported\n");
    exit(EXIT_FAILURE);
  }
  return cgroup2_mount_point;
}

int main(int argc, char *argv[]) {
  setuid(0);
  setgid(0);
  if (getuid() != 0 || getgid() != 0) {
    fprintf(stderr, "cgattach need suid sticky bit or run with root\n");
    exit(EXIT_FAILURE);
  }

  if (argc != 3) {
    fprintf(stderr, "only need 2 paramaters\n");
    print_usage();
    exit(EXIT_FAILURE);
  }

  string pid = string(argv[1]);
  string cgroup_target = string(argv[2]);
  validate(pid, cgroup_target);
  // string cgroup_mount_point = "/sys/fs/cgroup";
  string cgroup_mount_point = get_cgroup2_mount_point();
  string cgroup_target_path = cgroup_mount_point + cgroup_target;
  string cgroup_target_procs = cgroup_target_path + "/cgroup.procs";

  // check if exist, we will create it if not exist
  if (!exist(cgroup_target_path)) {
    if (mkdir(cgroup_target_path.c_str(),
              S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) {
      fprintf(stdout, "created cgroup %s success\n", cgroup_target.c_str());
    } else {
      fprintf(stderr, "created cgroup %s failed, errno %d\n",
              cgroup_target.c_str(), errno);
      exit(EXIT_FAILURE);
    }
    // fprintf(stderr, "cgroup %s not exist\n",cgroup_target.c_str());
    // exit(EXIT_FAILURE);
  }

  // put pid to target cgroup
  ofstream procs(cgroup_target_procs, ofstream::app);
  if (!procs.is_open()) {
    fprintf(stderr, "open file %s failed\n", cgroup_target_procs.c_str());
    exit(EXIT_FAILURE);
  }
  procs << pid.c_str() << endl;
  procs.close();

  // maybe there some write error, for example process pid may not exist
  if (!procs) {
    fprintf(stderr, "write %s to %s failed, maybe process %s not exist\n",
            pid.c_str(), cgroup_target_procs.c_str(), pid.c_str());
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

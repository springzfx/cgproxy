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


#define error(...) {fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}
#define debug(...) {fprintf(stdout, __VA_ARGS__);fprintf(stdout, "\n");}

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
 
  error("paramater validate error");
  print_usage();
  exit(EXIT_FAILURE);
}

string get_cgroup2_mount_point(){
  char cgroup2_mount_point[100]="";
  FILE* fp = popen("findmnt -t cgroup2 -n -o TARGET", "r");
  int count=fscanf(fp,"%s",&cgroup2_mount_point);
  fclose(fp);
  if (count=0){
    error("cgroup2 not supported");
    exit(EXIT_FAILURE);
  }
  return cgroup2_mount_point;
}

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
  validate(pid, cgroup_target);
  // string cgroup_mount_point = "/sys/fs/cgroup";
  string cgroup_mount_point = get_cgroup2_mount_point();
  string cgroup_target_path = cgroup_mount_point + cgroup_target;
  string cgroup_target_procs = cgroup_target_path + "/cgroup.procs";

  // check if exist, we will create it if not exist
  if (!exist(cgroup_target_path)) {
    if (mkdir(cgroup_target_path.c_str(),
              S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) {
      debug("created cgroup %s success", cgroup_target.c_str());
    } else {
      error("created cgroup %s failed, errno %d", cgroup_target.c_str(), errno);
      exit(EXIT_FAILURE);
    }
    // error("cgroup %s not exist",cgroup_target.c_str());
    // exit(EXIT_FAILURE);
  }

  // put pid to target cgroup
  ofstream procs(cgroup_target_procs, ofstream::app);
  if (!procs.is_open()) {
    error("open file %s failed", cgroup_target_procs.c_str());
    exit(EXIT_FAILURE);
  }
  procs << pid.c_str() << endl;
  procs.close();

  // maybe there some write error, for example process pid may not exist
  if (!procs) {
    error("write %s to %s failed, maybe process %s not exist",
            pid.c_str(), cgroup_target_procs.c_str(), pid.c_str());
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}

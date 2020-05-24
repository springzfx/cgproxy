#include "cgroup_attach.h"
#include "common.h"
#include <errno.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace CGPROXY::CGROUP {

string cgroup2_mount_point = get_cgroup2_mount_point();

bool exist(string path) {
  struct stat st;
  if (stat(path.c_str(), &st) != -1) { return S_ISDIR(st.st_mode); }
  return false;
}

string get_cgroup2_mount_point() {
  stringstream buffer;
  FILE *fp = popen("findmnt -t cgroup2 -n -o TARGET", "r");
  if (!fp) return "";
  char buf[64]; while (fgets(buf,64,fp)!=NULL) { buffer<<buf; }
  pclose(fp);
  string s=buffer.str();
  s.pop_back(); // remove newline character
  return s;
}

bool validate(string pid, string cgroup) {
  bool pid_v = validPid(pid);
  bool cg_v = validCgroup(cgroup);
  if (pid_v && cg_v) return true;

  error("attach paramater validate error");
  return_error;
}

int attach(const string pid, const string cgroup_target) {
  if (getuid() != 0) {
    error("need root to attach cgroup");
    return_error;
  }

  debug("attaching %s to %s", pid.c_str(), cgroup_target.c_str());

  int status;
  if (!validate(pid, cgroup_target)) return_error;
  if (cgroup2_mount_point.empty()) return_error;
  string cgroup_target_path = cgroup2_mount_point + cgroup_target;
  string cgroup_target_procs = cgroup_target_path + "/cgroup.procs";

  // check if exist, we will create it if not exist
  if (!exist(cgroup_target_path)) {
    if (mkdir(cgroup_target_path.c_str(),
              S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) {
      debug("created cgroup %s success", cgroup_target.c_str());
    } else {
      error("created cgroup %s failed, errno %d", cgroup_target.c_str(), errno);
      return_error;
    }
    // error("cgroup %s not exist",cgroup_target.c_str());
    // return_error
  }

  // put pid to target cgroup
  ofstream procs(cgroup_target_procs, ofstream::app);
  if (!procs.is_open()) {
    error("open file %s failed", cgroup_target_procs.c_str());
    return_error;
  }
  procs << pid.c_str() << endl;
  procs.close();

  // maybe there some write error, for example process pid may not exist
  if (!procs) {
    error("write %s to %s failed, maybe process %s not exist", pid.c_str(),
          cgroup_target_procs.c_str(), pid.c_str());
    return_error;
  }
  return_success;
}

int attach(const int pid, const string cgroup_target) {
  return attach(to_str(pid), cgroup_target);
}

} // namespace CGPROXY::CGROUP
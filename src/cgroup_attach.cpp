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
#include <thread>
#include <unistd.h>

namespace CGPROXY::CGROUP {

string cgroup2_mount_point = CGROUP2_MOUNT_POINT;


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

  if (!validate(pid, cgroup_target)) return_error;
  if (cgroup2_mount_point.empty()) return_error;
  string cgroup_target_path = cgroup2_mount_point + cgroup_target;
  string cgroup_target_procs = cgroup_target_path + "/cgroup.procs";

  // check if exist, we will create it if not exist
  if (!dirExist(cgroup_target_path)) {
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

  string cg;

  cg = getCgroup(pid);
  if (cg.empty()) return_success;
  if (cg == cgroup_target) {
    debug("%s already in %s", pid.c_str(), cgroup_target.c_str());
    return_success;
  }

  // put pid to target cgroup
  if (write2procs(pid, cgroup_target_procs) != 0) return_error;

  // wait for small period and check again
  this_thread::sleep_for(std::chrono::milliseconds(100));
  cg = getCgroup(pid);
  if (cg.empty()) return_success;
  if (cg != cgroup_target && write2procs(pid, cgroup_target_procs) != 0)
    return_error;
  return_success;
}

int write2procs(string pid, string procspath) {
  ofstream procs(procspath, ofstream::app);
  if (!procs.is_open()) {
    error("open file %s failed", procspath.c_str());
    return_error;
  }
  procs << pid.c_str() << endl;
  procs.close();

  // maybe there some write error, for example process pid may not exist
  if (!procs) {
    error("write %s to %s failed, maybe process %s live too short", pid.c_str(),
          procspath.c_str(), pid.c_str());
    return_error;
  }
  return_success;
}

int attach(const int pid, const string cgroup_target) {
  return attach(to_str(pid), cgroup_target);
}

} // namespace CGPROXY::CGROUP

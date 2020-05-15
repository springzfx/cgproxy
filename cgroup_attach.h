#ifndef CGPROUP_ATTACH_H
#define CGPROUP_ATTACH_H

#include <errno.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "common.h"
using namespace std;

namespace CGPROXY::CGROUP{

bool exist(string path) {
  struct stat st;
  if (stat(path.c_str(), &st) != -1) {
    return S_ISDIR(st.st_mode);
  }
  return false;
}

bool validate(string pid, string cgroup) {
  bool pid_v = validPid(pid);
  bool cg_v = validCgroup(cgroup);
  if (pid_v && cg_v)
    return true;
 
  error("attach paramater validate error");
  return_error
}

string get_cgroup2_mount_point(int &status){
  char cgroup2_mount_point[100]="";
  FILE* fp = popen("findmnt -t cgroup2 -n -o TARGET", "r");
  int count=fscanf(fp,"%s",&cgroup2_mount_point);
  fclose(fp);
  if (count=0){
    error("cgroup2 not supported");
    status=-1;
    return NULL;
  }
  status=0;
  return cgroup2_mount_point;
}

int attach(const string pid, const string cgroup_target) {
  if (getuid()!=0) {
    error("need root to attach cgroup");
    return_error
  }

  debug("attaching %s to %s",pid.c_str(),cgroup_target.c_str());
  
  int status;
  if (!validate(pid, cgroup_target)) return_error
  string cgroup_mount_point = get_cgroup2_mount_point(status);
  if (status!=0) return_error
  string cgroup_target_path = cgroup_mount_point + cgroup_target;
  string cgroup_target_procs = cgroup_target_path + "/cgroup.procs";

  // check if exist, we will create it if not exist
  if (!exist(cgroup_target_path)) {
    if (mkdir(cgroup_target_path.c_str(),
              S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) {
      debug("created cgroup %s success", cgroup_target.c_str());
    } else {
      error("created cgroup %s failed, errno %d", cgroup_target.c_str(), errno);
      return_error
    }
    // error("cgroup %s not exist",cgroup_target.c_str());
    // return_error
  }

  // put pid to target cgroup
  ofstream procs(cgroup_target_procs, ofstream::app);
  if (!procs.is_open()) {
    error("open file %s failed", cgroup_target_procs.c_str());
    return_error
  }
  procs << pid.c_str() << endl;
  procs.close();

  // maybe there some write error, for example process pid may not exist
  if (!procs) {
    error("write %s to %s failed, maybe process %s not exist",
            pid.c_str(), cgroup_target_procs.c_str(), pid.c_str());
    return_error
  }
  return_success
}

int attach(const int pid, const string cgroup_target){
  return attach(to_str(pid), cgroup_target);
}

}

#endif
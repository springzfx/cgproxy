#ifndef CGPROUP_ATTACH_H
#define CGPROUP_ATTACH_H

#include "common.hpp"
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
using namespace std;

namespace CGPROXY::CGROUP {

bool exist(string path);

bool validate(string pid, string cgroup);

string get_cgroup2_mount_point(int &status);

int attach(const string pid, const string cgroup_target);

int attach(const int pid, const string cgroup_target);

} // namespace CGPROXY::CGROUP

#endif
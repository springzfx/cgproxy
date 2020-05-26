#ifndef CGPROUP_ATTACH_H
#define CGPROUP_ATTACH_H

#include <stdlib.h>
#include <string>
using namespace std;

namespace CGPROXY::CGROUP {
extern string cgroup2_mount_point;
bool validate(string pid, string cgroup);
string get_cgroup2_mount_point();
int attach(const string pid, const string cgroup_target);
int attach(const int pid, const string cgroup_target);

} // namespace CGPROXY::CGROUP

#endif

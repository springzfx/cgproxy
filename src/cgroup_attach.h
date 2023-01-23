#ifndef CGPROUP_ATTACH_H
#define CGPROUP_ATTACH_H

#include <stdlib.h>
#include <string>

namespace CGPROXY::CGROUP {
extern const std::string cgroup2_mount_point;
bool validate(const std::string &pid, const std::string &cgroup);
int attach(const std::string &pid, const std::string &cgroup_target);
int attach(const int pid, const std::string &cgroup_target);
int write2procs(const std::string &pid, const std::string &procspath);

} // namespace CGPROXY::CGROUP

#endif

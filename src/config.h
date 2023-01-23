#ifndef CONFIG_H
#define CONFIG_H
#include "common.h"
#include <stdlib.h>
#include <string>
#include <vector>

namespace CGPROXY::CONFIG {

class Config {
public:
  const std::string cgroup_proxy_preserved = CGROUP_PROXY_PRESVERED;
  const std::string cgroup_noproxy_preserved = CGROUP_NOPROXY_PRESVERED;

  std::vector<std::string> program_proxy = {cgroup_proxy_preserved};
  std::vector<std::string> program_noproxy = {cgroup_noproxy_preserved};
  std::vector<std::string> cgroup_proxy;
  std::vector<std::string> cgroup_noproxy;
  bool enable_gateway = false;
  int port = 12345;
  bool enable_dns = true;
  bool enable_tcp = true;
  bool enable_udp = true;
  bool enable_ipv4 = true;
  bool enable_ipv6 = true;

  // for iptables
  int table=10007;
  int fwmark=0x9973;
  int mark_newin=0x9967;

  void toEnv() const;
  int saveToFile(const std::string &f);
  std::string toJsonStr();
  int loadFromFile(const std::string &f);
  int loadFromJsonStr(const std::string &js);
  void print_summary() const;

private:
  void mergeReserved();
  static bool validateJsonStr(const std::string &js);
  static void toRealProgramPath(std::vector<std::string> &v);
};

} // namespace CGPROXY::CONFIG
#endif

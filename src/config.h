#ifndef CONFIG_H
#define CONFIG_H
#include "common.h"
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;

namespace CGPROXY::CONFIG {

class Config {
public:
  const string cgroup_proxy_preserved = CGROUP_PROXY_PRESVERED;
  const string cgroup_noproxy_preserved = CGROUP_NOPROXY_PRESVERED;

  vector<string> program_proxy;
  vector<string> program_noproxy;
  vector<string> cgroup_proxy;
  vector<string> cgroup_noproxy;
  bool enable_gateway = false;
  int port = 12345;
  bool enable_dns = true;
  bool enable_tcp = true;
  bool enable_udp = true;
  bool enable_ipv4 = true;
  bool enable_ipv6 = true;

  void toEnv();
  int saveToFile(const string f);
  string toJsonStr();
  int loadFromFile(const string f);
  int loadFromJsonStr(const string js);
  void print_summary();

private:
  void mergeReserved();
  bool validateJsonStr(const string js);
  void toRealProgramPath(vector<string> &v);
};

} // namespace CGPROXY::CONFIG
#endif
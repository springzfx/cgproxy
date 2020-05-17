#ifndef CONFIG_H
#define CONFIG_H
#include "common.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
using json = nlohmann::json;

namespace CGPROXY::CONFIG {

class Config {
public:
  const string cgroup_proxy_preserved = CGROUP_PROXY_PRESVERED;
  const string cgroup_noproxy_preserved = CGROUP_NOPROXY_PRESVERED;

private:
  vector<string> cgroup_proxy;
  vector<string> cgroup_noproxy;
  bool enable_gateway = false;
  int port = 12345;
  bool enable_dns = true;
  bool enable_tcp = true;
  bool enable_udp = true;
  bool enable_ipv4 = true;
  bool enable_ipv6 = true;

public:
  void toEnv();
  int saveToFile(const string f);
  json toJson();
  int loadFromFile(const string f);
  int loadFromJson(const json &j);
  void mergeReserved();
  bool validateJson(const json &j);
};

} // namespace CGPROXY::CONFIG
#endif
#include "config.h"
#include "common.h"
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <set>
#include <vector>
using json = nlohmann::json;

#define add2json(v) j[#v] = v;
#define tryassign(v)                                                                     \
  try {                                                                                  \
    j.at(#v).get_to(v);                                                                  \
  } catch (exception & e) {}
#define merge(v)                                                                         \
  {                                                                                      \
    v.erase(std::remove(v.begin(), v.end(), v##_preserved), v.end());                    \
    v.insert(v.begin(), v##_preserved);                                                  \
  }

namespace CGPROXY::CONFIG {

void Config::toEnv() {
  setenv("cgroup_mount_point", CGROUP2_MOUNT_POINT, 1);
  setenv("program_proxy", join2str(program_proxy, ':').c_str(), 1);
  setenv("program_noproxy", join2str(program_noproxy, ':').c_str(), 1);
  setenv("cgroup_proxy", join2str(cgroup_proxy, ':').c_str(), 1);
  setenv("cgroup_noproxy", join2str(cgroup_noproxy, ':').c_str(), 1);
  setenv("enable_gateway", to_str(enable_gateway).c_str(), 1);
  setenv("port", to_str(port).c_str(), 1);
  setenv("enable_dns", to_str(enable_dns).c_str(), 1);
  setenv("enable_tcp", to_str(enable_tcp).c_str(), 1);
  setenv("enable_udp", to_str(enable_udp).c_str(), 1);
  setenv("enable_ipv4", to_str(enable_ipv4).c_str(), 1);
  setenv("enable_ipv6", to_str(enable_ipv6).c_str(), 1);
  setenv("table", to_str(table).c_str(), 1);
  setenv("fwmark", to_str(fwmark).c_str(), 1);
  setenv("mark_newin", to_str(mark_newin).c_str(), 1);
}

int Config::saveToFile(const string f) {
  ofstream o(f);
  if (!o.is_open()) return FILE_ERROR;
  string js = toJsonStr();
  o << setw(4) << js << endl;
  o.close();
  return 0;
}

string Config::toJsonStr() {
  json j;
  add2json(program_proxy);
  add2json(program_noproxy);
  add2json(cgroup_proxy);
  add2json(cgroup_noproxy);
  add2json(enable_gateway);
  add2json(port);
  add2json(enable_dns);
  add2json(enable_tcp);
  add2json(enable_udp);
  add2json(enable_ipv4);
  add2json(enable_ipv6);
  add2json(table);
  add2json(fwmark);
  add2json(mark_newin);
  return j.dump();
}

int Config::loadFromFile(const string f) {
  debug("loading config: %s", f.c_str());
  ifstream ifs(f);
  if (ifs.is_open()) {
    string js = to_str(ifs.rdbuf());
    ifs.close();
    return loadFromJsonStr(js);
  } else {
    error("open failed: %s", f.c_str());
    return FILE_ERROR;
  }
}

int Config::loadFromJsonStr(const string js) {
  if (!validateJsonStr(js)) {
    error("json validate fail");
    return PARAM_ERROR;
  }
  json j = json::parse(js);
  tryassign(program_proxy);
  tryassign(program_noproxy);
  tryassign(cgroup_proxy);
  tryassign(cgroup_noproxy);
  tryassign(enable_gateway);
  tryassign(port);
  tryassign(enable_dns);
  tryassign(enable_tcp);
  tryassign(enable_udp);
  tryassign(enable_ipv4);
  tryassign(enable_ipv6);
  tryassign(table);
  tryassign(fwmark);
  tryassign(mark_newin);

  // e.g. v2ray -> /usr/bin/v2ray -> /usr/lib/v2ray/v2ray
  toRealProgramPath(program_noproxy);
  toRealProgramPath(program_proxy);

  mergeReserved();

  return 0;
}

void Config::mergeReserved() {
  merge(cgroup_proxy);
  merge(cgroup_noproxy);
}

bool Config::validateJsonStr(const string js) {
  json j = json::parse(js);
  bool status = true;
  const set<string> boolset = {"enable_gateway", "enable_dns",  "enable_tcp",
                               "enable_udp",     "enable_ipv4", "enable_ipv6"};
  const set<string> allowset = {"program_proxy", "program_noproxy", "comment", "table", "fwmark", "mark_newin"};
  for (auto &[key, value] : j.items()) {
    if (key == "cgroup_proxy" || key == "cgroup_noproxy") {
      if (value.is_string() && !validCgroup((string)value)) status = false;
      // TODO what if vector<int> etc.
      if (value.is_array() && !validCgroup((vector<string>)value)) status = false;
      if (!value.is_string() && !value.is_array()) status = false;
    } else if (key == "port") {
      if (!validPort(value)) status = false;
    } else if (boolset.find(key) != boolset.end()) {
      if (!value.is_boolean()) status = false;
    } else if (allowset.find(key) != allowset.end()) {

    } else {
      error("unknown key: %s", key.c_str());
      return false;
    }
    if (!status) {
      error("invalid value for key: %s", key.c_str());
      return false;
    }
  }
  return true;
}

void Config::print_summary() {
  info("noproxy program: %s", join2str(program_noproxy).c_str());
  info("proxied program: %s", join2str(program_proxy).c_str());
  info("noproxy cgroup: %s", join2str(cgroup_noproxy).c_str());
  info("proxied cgroup: %s", join2str(cgroup_proxy).c_str());
  info("table: %d, fwmark: %d, mark_newin: %d", table, fwmark, mark_newin);
}

void Config::toRealProgramPath(vector<string> &v) {
  vector<string> tmp;
  for (auto &p : v) {
    auto rpath = getRealExistPath(p);
    if (!rpath.empty()) tmp.push_back(rpath);
    else
      warning("%s not exist or broken link", p.c_str());
  }
  v = tmp;
}

#undef tryassign
#undef add2json
#undef merge

} // namespace CGPROXY::CONFIG
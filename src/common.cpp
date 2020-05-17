#include "common.hpp"

bool enable_debug = false;

string join2str(const vector<string> t, const char delm) {
  string s;
  for (const auto &e : t) e != *(t.end() - 1) ? s += e + delm : s += e;
  return s;
}

string join2str(const int argc, char **argv, const char delm) {
  string s;
  for (int i = 0; i < argc; i++) {
    s += argv[i];
    if (i != argc - 1) s += delm;
  }
  return s;
}

bool validCgroup(const string cgroup) {
  return regex_match(cgroup, regex("^/[a-zA-Z0-9\\-_./@]*$"));
}

bool validCgroup(const vector<string> cgroup) {
  for (auto &e : cgroup) {
    if (!regex_match(e, regex("^/[a-zA-Z0-9\\-_./@]*$"))) { return false; }
  }
  return true;
}

bool validPid(const string pid) { return regex_match(pid, regex("^[0-9]+$")); }

bool validPort(const int port) { return port > 0; }

#include "common.h"
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

bool enable_debug = false;
bool enable_info = true;

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

bool startWith(string s, string prefix) { return s.rfind(prefix, 0) == 0; }

bool validCgroup(const string cgroup) {
  return regex_match(cgroup, regex("^/[a-zA-Z0-9\\-_./@]*$"));
}

bool validCgroup(const vector<string> cgroup) {
  for (auto &e : cgroup) {
    if (!regex_match(e, regex("^/[a-zA-Z0-9\\-_./@]*$"))) { return false; }
  }
  return true;
}

bool validIpv4(const string ip) {
  return regex_match(ip, regex(R"((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5]))"));
}

bool validIpv4(const vector<string> ip) {
  for (auto &e : ip) {
    if (!regex_match(e, regex(R"((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5]))"))) { return false; }
  }
  return true;
}

bool validIpv6(const string ip) {
  return regex_match(ip, regex(R"(\[\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*\])"));
}

bool validIpv6(const vector<string> ip) {
  for (auto &e : ip) {
    if (!regex_match(e, regex(R"(\[\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*\])"))) { return false; }
  }
  return true;
}

bool validPid(const string pid) { return regex_match(pid, regex("^[0-9]+$")); }

bool validPort(const int port) { return port > 0; }

bool fileExist(const string &path) {
  struct stat st;
  return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

bool dirExist(const string &path) {
  struct stat st;
  return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

vector<int> bash_pidof(const string &path) {
  vector<int> pids;
  unique_ptr<FILE, decltype(&pclose)> fp(popen(to_str("pidof ", path).c_str(), "r"),
                                         &pclose);
  if (!fp) return pids;
  int pid;
  while (fscanf(fp.get(), "%d", &pid) != EOF) { pids.push_back(pid); }
  return pids;
}

string bash_which(const string &name) {
  stringstream buffer;
  unique_ptr<FILE, decltype(&pclose)> fp(popen(to_str("which ", name).c_str(), "r"),
                                         &pclose);
  if (!fp) return "";
  char buf[READ_SIZE_MAX];
  while (fgets(buf, READ_SIZE_MAX, fp.get()) != NULL) { buffer << buf; }
  string s = buffer.str();
  if (!s.empty()) s.pop_back(); // remove newline character
  return s;
}

string bash_readlink(const string &path) {
  stringstream buffer;
  unique_ptr<FILE, decltype(&pclose)> fp(popen(to_str("readlink -e ", path).c_str(), "r"),
                                         &pclose);
  if (!fp) return "";
  char buf[READ_SIZE_MAX];
  while (fgets(buf, READ_SIZE_MAX, fp.get()) != NULL) { buffer << buf; }
  string s = buffer.str();
  if (!s.empty()) s.pop_back(); // remove newline character
  return s;
}

string getRealExistPath(const string &name) {
  if (name[0] == '/' && fileExist(name)) return name;
  string path;
  path = bash_which(name);
  if (path.empty()) return "";
  path = bash_readlink(path);
  if (!fileExist(path)) return "";
  return path;
}

bool belongToCgroup(string cg1, string cg2) { return startWith(cg1 + '/', cg2 + '/'); }

bool belongToCgroup(string cg1, vector<string> cg2) {
  for (const auto &s : cg2) {
    if (startWith(cg1 + '/', s + '/')) return true;
  }
  return false;
}

string getCgroup(const pid_t &pid) { return getCgroup(to_str(pid)); }

string getCgroup(const string &pid) {
  string cgroup_f = to_str("/proc/", pid, "/cgroup");
  if (!fileExist(cgroup_f)) return "";

  string cgroup, line;
  ifstream ifs(cgroup_f);
  debug("prcessing file %s", cgroup_f.c_str());
  while (ifs.good() && getline(ifs, line)) {
    // debug("process line: %s", line.c_str());
    if (line[0] == '0') {
      cgroup = line.substr(3);
      debug("get cgroup of %s: %s", pid.c_str(), cgroup.c_str());
      break;
    }
  }
  ifs.close();
  return cgroup;
}

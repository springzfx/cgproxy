#include "common.h"
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

bool enable_debug = false;
bool enable_info = true;

string join2str(const vector<string> &t, const char delm) {
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

bool startWith(const string &s, const string &prefix) { return s.rfind(prefix, 0) == 0; }

bool validCgroup(const string &cgroup) {
  return regex_match(cgroup, regex("^/[a-zA-Z0-9\\-_./@]*$"));
}

bool validCgroup(const vector<string> &cgroup) {
  return std::all_of(cgroup.cbegin(), cgroup.cend(), [](auto &e) { return regex_match(e, regex("^/[a-zA-Z0-9\\-_./@]*$")); });
}

bool validPid(const string &pid) { return regex_match(pid, regex("^[0-9]+$")); }

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
  unique_ptr<FILE, decltype(&pclose)> const fp(popen(to_str("pidof ", path).c_str(), "r"),
                                         &pclose);
  if (!fp) return pids;
  int pid;
  while (fscanf(fp.get(), "%d", &pid) != EOF) { pids.push_back(pid); }
  return pids;
}

string bash_which(const string &name) {
  stringstream buffer;
  unique_ptr<FILE, decltype(&pclose)> const fp(popen(to_str("which ", name).c_str(), "r"),
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
  unique_ptr<FILE, decltype(&pclose)> const fp(popen(to_str("readlink -e ", path).c_str(), "r"),
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

bool belongToCgroup(const string &cg1, const string &cg2) { return startWith(cg1 + '/', cg2 + '/'); }

bool belongToCgroup(const string &cg1, const vector<string> &cg2) {
  return std::any_of(cg2.cbegin(), cg2.cend(), [&](auto s) { return startWith(cg1 + '/', s + '/'); });
}

string getCgroup(const pid_t &pid) { return getCgroup(to_str(pid)); }

string getCgroup(const string &pid) {
  const string cgroup_f = to_str("/proc/", pid, "/cgroup");
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

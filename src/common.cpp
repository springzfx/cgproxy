#include "common.h"
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

bool enable_debug = false;
bool enable_info = true;

std::string join2str(const std::vector<std::string> &t, const char delm) {
  std::string s;
  for (const auto &e : t) e != *(t.end() - 1) ? s += e + delm : s += e;
  return s;
}

std::string join2str(const int argc, char **argv, const char delm) {
  std::string s;
  for (int i = 0; i < argc; i++) {
    s += argv[i];
    if (i != argc - 1) s += delm;
  }
  return s;
}

bool startWith(const std::string &s, const std::string &prefix) { return s.rfind(prefix, 0) == 0; }

bool validCgroup(const std::string &cgroup) {
  return std::regex_match(cgroup, std::regex("^/[a-zA-Z0-9\\-_./@]*$"));
}

bool validCgroup(const std::vector<std::string> &cgroup) {
  return std::all_of(cgroup.cbegin(), cgroup.cend(), [](auto &e) { return std::regex_match(e, std::regex("^/[a-zA-Z0-9\\-_./@]*$")); });
}

bool validPid(const std::string &pid) { return std::regex_match(pid, std::regex("^[0-9]+$")); }

bool validPort(const int port) { return port > 0; }

bool fileExist(const std::string &path) {
  struct stat st;
  return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

bool dirExist(const std::string &path) {
  struct stat st;
  return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

std::vector<int> bash_pidof(const std::string &path) {
  std::vector<int> pids;
  std::unique_ptr<FILE, decltype(&pclose)> const fp(popen(to_str("pidof ", path).c_str(), "r"),
                                         &pclose);
  if (!fp) return pids;
  int pid;
  while (fscanf(fp.get(), "%d", &pid) != EOF) { pids.push_back(pid); }
  return pids;
}

std::string bash_which(const std::string &name) {
  std::stringstream buffer;
  std::unique_ptr<FILE, decltype(&pclose)> const fp(popen(to_str("which ", name).c_str(), "r"),
                                         &pclose);
  if (!fp) return "";
  char buf[READ_SIZE_MAX];
  while (fgets(buf, READ_SIZE_MAX, fp.get()) != NULL) { buffer << buf; }
  std::string s = buffer.str();
  if (!s.empty()) s.pop_back(); // remove newline character
  return s;
}

std::string bash_readlink(const std::string &path) {
  std::stringstream buffer;
  std::unique_ptr<FILE, decltype(&pclose)> const fp(popen(to_str("readlink -e ", path).c_str(), "r"),
                                         &pclose);
  if (!fp) return "";
  char buf[READ_SIZE_MAX];
  while (fgets(buf, READ_SIZE_MAX, fp.get()) != NULL) { buffer << buf; }
  std::string s = buffer.str();
  if (!s.empty()) s.pop_back(); // remove newline character
  return s;
}

std::string getRealExistPath(const std::string &name) {
  if (name[0] == '/' && fileExist(name)) return name;
  std::string path;
  path = bash_which(name);
  if (path.empty()) return "";
  path = bash_readlink(path);
  if (!fileExist(path)) return "";
  return path;
}

bool belongToCgroup(const std::string &cg1, const std::string &cg2) { return startWith(cg1 + '/', cg2 + '/'); }

bool belongToCgroup(const std::string &cg1, const std::vector<std::string> &cg2) {
  return std::any_of(cg2.cbegin(), cg2.cend(), [&](auto s) { return startWith(cg1 + '/', s + '/'); });
}

std::string getCgroup(const pid_t &pid) { return getCgroup(to_str(pid)); }

std::string getCgroup(const std::string &pid) {
  const std::string cgroup_f = to_str("/proc/", pid, "/cgroup");
  if (!fileExist(cgroup_f)) return "";

  std::string cgroup, line;
  std::ifstream ifs(cgroup_f);
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

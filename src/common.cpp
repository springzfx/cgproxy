#include "common.h"
#include <fstream>
#include <limits.h>
#include <linux/limits.h>
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
  FILE *fp = popen(to_str("pidof ", path).c_str(), "r");
  if (!fp) return pids;
  int pid;
  char buf[64];
  while (fscanf(fp, "%d", &pid) != EOF) { pids.push_back(pid); }
  pclose(fp);
  return pids;
}

string bash_which(const string &name) {
  stringstream buffer;
  FILE *fp = popen(to_str("which ", name).c_str(), "r");
  if (!fp) return "";
  char buf[64];
  while (fgets(buf, 64, fp) != NULL) { buffer << buf; }
  pclose(fp);
  string s = buffer.str();
  s.pop_back(); // remove newline character
  return s;
}

string bash_readlink(const string &path) {
  stringstream buffer;
  FILE *fp = popen(to_str("readlink -e ", path).c_str(), "r");
  if (!fp) return "";
  char buf[64];
  while (fgets(buf, 64, fp) != NULL) { buffer << buf; }
  pclose(fp);
  string s = buffer.str();
  s.pop_back(); // remove newline character
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
#include "common.h"
#include "config.h"
#include "socket_client.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
using namespace CGPROXY;
using namespace CGPROXY::CONFIG;

void send_config(Config &config, int &status) {
  json j;
  j["type"] = MSG_TYPE_CONFIG_JSON;
  j["data"] = config.toJsonStr();
  SOCKET::send(j.dump(), status);
}

void send_config_path(const string s, int &status) {
  json j;
  j["type"] = MSG_TYPE_CONFIG_PATH;
  j["data"] = s;
  SOCKET::send(j.dump(), status);
}

void send_pid(const pid_t pid, bool proxy, int &status) {
  json j;
  j["type"] = proxy ? MSG_TYPE_PROXY_PID : MSG_TYPE_NOPROXY_PID;
  j["data"] = pid;
  SOCKET::send(j.dump(), status);
}

void test_config() {
  Config config;
  config.cgroup_proxy = {"/"};
  int status;
  send_config(config, status);
}

void test_config_path() {
  string path = "/etc/cgproxy/config.json";
  int status;
  send_config_path(path, status);
}

int main() {
  test_config();
  return 0;
}
#include "socket_client.hpp"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
using namespace CGPROXY;

void test_json() {
  json j;
  j["type"] = MSG_TYPE_JSON;
  j["data"]["cgroup_proxy"] = "/";
  j["data"]["enable_dns"] = false;
  int status;
  SOCKET::send(j.dump(), status);
}

void test_json_array() {
  json j;
  j["type"] = MSG_TYPE_JSON;
  j["data"]["cgroup_proxy"] = "/proxy.slice";
  j["data"]["cgroup_noproxy"] = {"/noproxy.slice", "/system.slice/v2ray.service"};
  int status;
  SOCKET::send(j.dump(), status);
}

void test_file() {
  json j;
  j["type"] = MSG_TYPE_CONFIG_PATH;
  j["data"] = "/etc/cgproxy.conf";
  int status;
  SOCKET::send(j.dump(), status);
}

void test_pid() {
  json j;
  j["type"] = MSG_TYPE_PROXY_PID;
  j["data"] = "9999";
  int status;
  SOCKET::send(j.dump(), status);
}

int main() {
  test_json_array();
  test_file();
  test_json();
  test_pid();
  return 0;
}
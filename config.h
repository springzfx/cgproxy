#ifndef CONFIG_H
#define CONFIG_H
#include "common.h"
#include "socket_server.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <set>
#include <iomanip>
using namespace std;
using json = nlohmann::json;

namespace CGPROXY::CONFIG{

struct Config {
  public:
  const string cgroup_proxy_preserved=CGROUP_PROXY_PRESVERED;
  const string cgroup_noproxy_preserved=CGROUP_NOPROXY_PRESVERED;
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
  void toEnv() {
    mergeReserved();
    setenv("cgroup_proxy", join2str(cgroup_proxy).c_str(), 1);
    setenv("cgroup_noproxy", join2str(cgroup_noproxy).c_str(), 1);
    setenv("enable_gateway", to_str(enable_gateway).c_str(), 1);
    setenv("port", to_str(port).c_str(), 1);
    setenv("enable_dns", to_str(enable_dns).c_str(), 1);
    setenv("enable_tcp", to_str(enable_tcp).c_str(), 1);
    setenv("enable_udp", to_str(enable_udp).c_str(), 1);
    setenv("enable_ipv4", to_str(enable_ipv4).c_str(), 1);
    setenv("enable_ipv6", to_str(enable_ipv6).c_str(), 1);
  }

  int saveToFile(const string f){
      ofstream o(f);
      if (!o.is_open()) return FILE_ERROR;
      json j=toJson();
      o << setw(4) << j << endl;
      o.close();
      return 0;
  }

  json toJson(){
      json j;
      #define add2json(v) j[#v]=v;
      add2json(cgroup_proxy);
      add2json(cgroup_noproxy);
      add2json(enable_gateway);
      add2json(port);
      add2json(enable_dns);
      add2json(enable_tcp);
      add2json(enable_udp);
      add2json(enable_ipv4);
      add2json(enable_ipv6);
      #undef add2json    
      return j; 
  }

  int loadFromFile(const string f) {
    debug("loading config: %s", f.c_str());
    ifstream ifs(f);
    if (ifs.is_open()){
      json j;
      try { ifs >> j; }catch (exception& e){error("parse error: %s", f.c_str());ifs.close();return PARSE_ERROR;}
      ifs.close();
      return loadFromJson(j);
    }else{
      error("open failed: %s",f.c_str());
      return FILE_ERROR;
    }
  }

  int loadFromJson(const json &j) {
    if (!validateJson(j)) {error("json validate fail"); return PARAM_ERROR;}
    #define tryassign(v) try{j.at(#v).get_to(v);}catch(exception &e){}
    tryassign(cgroup_proxy);
    tryassign(cgroup_noproxy);
    tryassign(enable_gateway);
    tryassign(port);
    tryassign(enable_dns);
    tryassign(enable_tcp);
    tryassign(enable_udp);
    tryassign(enable_ipv4);
    tryassign(enable_ipv6);
    #undef assign
    return 0;
  }

  void mergeReserved(){
    #define merge(v) { \
      v.erase(std::remove(v.begin(), v.end(), v ## _preserved), v.end()); \
      v.insert(v.begin(), v ## _preserved); \
      }
    merge(cgroup_proxy);
    merge(cgroup_noproxy);
    #undef merge
    
  }

  bool validateJson(const json &j){
    bool status=true;
    const set<string> boolset={"enable_gateway","enable_dns","enable_tcp","enable_udp","enable_ipv4","enable_ipv6"};
    for (auto& [key, value] : j.items()) {
      if (key=="cgroup_proxy"||key=="cgroup_noproxy"){
        if (value.is_string()&&!validCgroup((string)value)) status=false;
        if (value.is_array()&&!validCgroup((vector<string>)value)) status=false;
        if (!value.is_string()&&!value.is_array()) status=false;
      }else if (key=="port"){
        if (validPort(value)) status=false;
      }else if (boolset.find(key)!=boolset.end()){
        if (value.is_boolean()) status=false;
      }else{
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
};

}
#endif
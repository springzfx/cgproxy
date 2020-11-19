

# Transparent Proxy powered by cgroup v2



## Introduction

cgproxy will transparent proxy anything running in specific cgroup. It resembles with *proxychains* and *tsock*s in default setting. 

Main feature:

- supports cgroup/program level proxy control. 
- supports global transparent proxy and gateway proxy. 

## Contents

<!--ts-->
   * [Transparent Proxy powered by cgroup v2](#transparent-proxy-powered-by-cgroup-v2)
      * [Introduction](#introduction)
      * [Contents](#contents)
      * [Prerequest](#prerequest)
      * [How to build and install](#how-to-build-and-install)
      * [Default usage](#default-usage)
      * [Configuration](#configuration)
      * [Global transparent proxy](#global-transparent-proxy)
      * [Gateway proxy](#gateway-proxy)
      * [Other useful tools provided in this project](#other-useful-tools-provided-in-this-project)
      * [NOTES](#notes)
      * [TIPS](#tips)
      * [Licences](#licences)
      * [Known Issues](#known-issues)

<!-- Added by: fancy, at: Sat 04 Jul 2020 03:52:07 PM CST -->

<!--te-->

## Prerequest

- cgroup2

  Both cgroup and cgroup2 are enabled in linux by default. So you don't have to do anything about this.
  - `systemd-cgls` to see the cgroup hierarchical tree.
  - Why cgroup v2?  Because simple, elegant and intuitive.

- TPROXY

  A process listening on port (e.g.  12345)  to accept iptables TPROXY, for example v2ray's dokodemo-door in tproxy mode.

- Iptables

  Iptables version should be at least 1.6.0, run `iptables --version` to check.

  ubuntu 16.04, debian 9, fedora 27 and later are desired

## How to build and install

### distro install

- For debian and redhat series, download from [Release page](https://github.com/springzfx/cgproxy/releases)

- For archlinux series, already in archlinuxcn repo, or see [archlinux AUR](https://aur.archlinux.org/packages/?K=cgproxy)

- **Tested on  archlinux, fedora 32, ubuntu 18.04, ubuntu 20.04,  deepin 15.11, deepin v20 beta**

### build

- before build, install depencies: clang(if to build bpf obj from scratch), nlohmann-json, libbpf
- then cmake standard build

```bash
# ready build dir
mkdir build
cd build
# generate
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Dbuild_execsnoop_dl=ON \
      -Dbuild_static=OFF \
      ..
# compile
make
```

## Default usage

- First enable and start service

  ```bash
  sudo systemctl enable --now cgproxy.service
  ```
  
- Then prefix with cgproxy with your command, just like proxychains

  ```bash
  cgproxy [--debug] <CMD>
  ```

- For example, test proxy

  ```bash
  cgproxy curl -vI https://www.google.com
  ```

- To completely stop
  ```
  sudo systemctl disable --now cgproxy.service
  ```

## Configuration

Config file: **/etc/cgproxy/config.json**

```json
{
    "port": 12345,
    "program_noproxy": ["v2ray", "qv2ray"],
    "program_proxy": [],
    "cgroup_noproxy": ["/system.slice/v2ray.service"],
    "cgroup_proxy": [],
    "enable_gateway": false,
    "enable_dns": true,
    "enable_udp": true,
    "enable_tcp": true,
    "enable_ipv4": true,
    "enable_ipv6": true,
    "table": 10007,
    "fwmark": 39283
}

```

- **port** tproxy listenning port

- program level proxy control, need execsnoop enabled:

  - **program_proxy**  program need to be proxied
  - **program_noproxy** program that won't be proxied

- cgroup level proxy control:

  - **cgroup_noproxy** cgroup array that no need to proxy, `/noproxy.slice` is preserved
  - **cgroup_proxy** cgroup array that need to proxy, `/proxy.slice` is preserved

- **enable_gateway** enable gateway proxy for local devices

- **enable_dns** enable dns to go to proxy

- **enable_tcp**

- **enable_udp**

- **enable_ipv4**

- **enable_ipv6**

- **table**, **fwmark** you can specify iptables and route table related parameter in case conflict.

- options priority

  ```
  program_noproxy > program_proxy > cgroup_noproxy > cgroup_proxy
  enable_ipv6 = enable_ipv4 > enable_dns > enable_tcp = enable_udp
  command cgproxy and cgnoproxy always have highest priority
  ```

**Note**: cgroup in configuration need to be exist, otherwise ignored

If you changed config, remember to restart service

```bash
sudo systemctl restart cgproxy.service
```

## Global transparent proxy

- Set `"cgroup_proxy":["/"]`  in configuration, this will proxy all connection

- Allow your proxy program (v2ray) direct to internet to avoid loop. Two ways:
  
  - active way, run command
    
    example: `cgnoproxy sudo v2ray -config config_file`
    
    example: `cgnoproxy qv2ray`
    
  - passive way,  persistent config
  
      example:  `"program_noproxy":["v2ray" ,"qv2ray"]`
      
      example:  `"cgroup_noproxy":["/system.slice/v2ray.service"]`
  
- Finally, restart cgproxy service, that's all

## Gateway proxy

- Set `"enable_gateway":true` in configuration
- And allow your proxy software (v2ray) direct to internet if necessary, described above
- Other device set this host as gateway, and set public dns if need

## Other useful tools provided in this project

- `cgnoproxy` run program wihout proxy, very useful in global transparent proxy

  ```bash
  cgnoproxy [--debug] <CMD>
  cgnoproxy [--debug] --pid <PID>
  ```
  
- For more detail command usage, see `man cgproxyd`  `man cgproxy`  `man cgnoproxy` 

## NOTES

- v2ray TPROXY need root or special permission, use [service](/v2ray_config/v2ray.service) or
  
  ```bash
  sudo setcap "cap_net_admin,cap_net_bind_service=ep" /usr/bin/v2ray
  ```

- Why not outbound mark solution, because in v2ray [when `"localhost"` is used, out-going DNS traffic is not controlled by V2Ray](https://www.v2fly.org/config/dns.html#dnsobject), so no mark at all, that's pity.

## TIPS

- `systemd-cgls` to see the cgroup hierarchical tree.
- Check cgroup2 support `findmnt -t cgroup2`
- Offer you v2ray service and full config exmaple in [v2ray_config](https://github.com/springzfx/cgproxy/tree/master/v2ray_config)
- Offer you qv2ray config example
  

![Qv2ray config example](https://i.loli.net/2020/08/17/P6y5SfLoUwGjaxM.png)

## Licences

cgproxy is licenced under [![License: GPL v3](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/gpl-2.0) 

## Known Issues

- docker breaks cgroup v2 path match, add kernel parameter `cgroup_no_v1=net_cls,net_prio` to resolve, see [issue #3](https://github.com/springzfx/cgproxy/issues/3) for detail

- docker load `br_netfilter` module due to [hairpin nat](https://wiki.mikrotik.com/wiki/Hairpin_NAT),  which is not a big deal,  see [commit](https://github.com/moby/moby/pull/13162). 

  It enables data link layer packet to go through iptables and only once. However TPROXY do not accept this kind of packets. So to get it working, set following parameter to disable this behavior or unload br_netfilter module manualy. see [issue #10](https://github.com/springzfx/cgproxy/issues/10) for detail.

  ```
  sudo sysctl -w net.bridge.bridge-nf-call-iptables=0
  sudo sysctl -w net.bridge.bridge-nf-call-ip6tables=0
  sudo sysctl -w net.bridge.bridge-nf-call-arptables = 0
  ```

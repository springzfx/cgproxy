

# Transparent Proxy with cgroup v2



## Introduction

cgproxy will transparent proxy anything running in specific cgroup. It resembles with *proxychains* and *tsock*, but without their disadvantages, and more powerfull.

It aslo supports global transparent proxy and gateway proxy. See [Global transparent proxy](#global-transparent-proxy) and  [Gateway proxy](#gateway-proxy)

<!--ts-->

   * [Transparent Proxy with cgroup v2](#transparent-proxy-with-cgroup-v2)
      * [Introduction](#introduction)
      * [Prerequest](#prerequest)
      * [How to install](#how-to-install)
      * [How to use](#how-to-use)
      * [Global transparent proxy](#global-transparent-proxy)
      * [Gateway proxy](#gateway-proxy)
      * [Other useful tools provided in this project](#other-useful-tools-provided-in-this-project)
      * [NOTES](#notes)
      * [TIPS](#tips)
      * [Licences](#licences)

<!-- Added by: fancy, at: Thu 23 Apr 2020 01:23:57 PM HKT -->

<!--te-->

## Prerequest

- cgroup2

  Both cgroup and cgroup2 are enabled in linux by default. So you don't have to do anything about this.
  - `systemd-cgls` to see the cgroup hierarchical tree.
  - Why cgroup v2?  Because simple, elegant and intuitive.

- TPROXY

  A process listening on port (e.g.  12345)  to accept iptables TPROXY, for example v2ray's dokodemo-door in tproxy mode.

## How to install

```bash
mkdir build && cd build && cmake .. && make && sudo make install
```

- It is alreay in [archlinux AUR](https://aur.archlinux.org/packages/cgproxy/). 

- DEB and RPM are packaged in [release page](https://github.com/springzfx/cgproxy/releases).

## How to use

- First enable and start service

  ```bash
  sudo systemctl enable --now cgproxy.service
  ```
  
- Then prefix with cgproxy with your command, just like proxychains

  ```
  cgproxy <CMD>
  ```

- For example, test proxy

  ```bash
  cgproxy curl -vIs https://www.google.com
  ```

- To completely stop
  ```
  sudo systemctl disable --now cgproxy.service
  ```
----
<details>
  <summary>More config in <i>/etc/cgproxy.conf</i>  (click to expand)</summary>

```bash
########################################################################
## cgroup transparent proxy
## any process in cgroup_proxy will be proxied, and cgroup_noproxy the opposite
## cgroup must start with slash '/'
# cgroup_proxy="/"
# cgroup_noproxy="/system.slice/v2ray.service"
cgroup_proxy="/proxy.slice"
cgroup_noproxy="/noproxy.slice"

########################################################################
## allow as gateway for local network
enable_gateway=false

########################################################################
## listening port of another proxy process, for example v2ray 
port=12345

########################################################################
## if you set to false, it's traffic won't go through proxy, but still can go direct to internet 
enable_tcp=true
enable_udp=true
enable_ipv4=true
enable_ipv6=true
enable_dns=true

########################################################################
## do not modify this if you don't known what you are doing
table=100
fwmark=0x01
mark_noproxy=0xff
mark_newin=0x02
```
</details>
If you changed config, remember to restart service

```bash
sudo systemctl restart cgproxy.service
```

## Global transparent proxy

- Set `cgroup_proxy="/"`  in */etc/cgproxy.conf*, this will proxy all connection

- And allow your proxy program (v2ray) direct to internet, two ways:
  - active way

      run `cgnoproxy <PROXY PROGRAM>`
      
      example: `cgnoproxy sudo v2ray -config config_file`
      
  - passive way
  
      set `cgroup_noproxy="<PROXY PROGRAM's CGROUP>"`
  
      example:  `cgroup_noproxy="/system.slice/v2ray.service"`
  
- Finally, restart cgproxy service, that's all

## Gateway proxy

- Set `enable_gateway=true` in */etc/cgproxy.conf*
- And allow your proxy software (v2ray) direct to internet, described above
- Other device set this host as gateway, and set public dns if necessary

## Other useful tools provided in this project

- `cgnoproxy` run program wihout proxy, very useful in global transparent proxy

  ```bash
  cgnoproxy <CMD> 
  ```
  
- `run_in_cgroup` run command in specific cgroup which will create if not exist , cgroup can be only one level down exist cgroup, otherwise created fail.

  ```bash
  run_in_cgroup --cgroup=CGROUP <COMMAND>
  # example
  run_in_cgroup --cgroup=/mycgroup.slice ping 127.0.0.1
  ```
  
- `cgattach` attach specific process pid to specific cgroup which will create if not exist , cgroup can be only one level down exist cgroup, otherwise created fail.

  ```bash
  cgattch <pid> <cgroup>
  # example
  cgattch 9999 /proxy.slice
  ```

## NOTES

- `cgattach` has *suid* bit set by default, be careful to use on multi-user server for securiry. To avoid this situation,  you can remove the *suid* bit , then it will fallback to use *sudo*, with *sudoer* you can restrict permission or set NOPASSWD for youself.

- v2ray TPROXY need root or special permission
  
  ```bash
  sudo setcap "cap_net_admin,cap_net_bind_service=ep" /usr/lib/v2ray/v2ray
  ```

- Why not outbound mark solution, because in v2ray [when `"localhost"` is used, out-going DNS traffic is not controlled by V2Ray](https://www.v2fly.org/en/configuration/dns.html), so no mark at all, that's pity.

## TIPS

- `systemd-cgls` to see the cgroup hierarchical tree.
- Check cgroup2 support `findmnt -t cgroup2`
- Offer you v2ray service and full config exmaple in [v2ray_config](https://github.com/springzfx/cgproxy/tree/master/v2ray_config)
- Offer you qv2ray config example
  

![Qv2ray config example](https://i.loli.net/2020/04/28/bdQBzUD37FOgfvt.png)

## Licences

cgproxy is licenced under [![License: GPL v3](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/gpl-2.0) 

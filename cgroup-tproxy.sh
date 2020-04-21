#!/bin/bash
print_help(){
cat << 'DOC'
#############################################################################
# 
# 1. For now, linux default using cgroup v1 for compatibility
#    this script need cgroup v2, you need enable cgroup v2 in your system. 
# 
# 2. Listening port is expected to accept iptables TPROXY, while REDIRECT 
#    will not work in this script, because REDIRECT only support tcp/ipv4
# 
# 3. TPROXY need root or cap_net_admin capability whatever process is listening on port
#    v2ray as example: sudo setcap cap_net_admin+ep /usr/lib/v2ray/v2ray
# 
# 4. this script will proxy anything running in specific cgroup
# 
# script usage:
#     cgroup-tproxy.sh [--help|--config|stop]
#     --config=FILE
#           load config from file
#     --help
#           show help info
#     stop
#           clean then stop
# 
# proxy usage:
#     cgproxy <program>
# 
#############################################################################
DOC
}

## any process in this cgroup will be proxied
proxy_cgroup="/user.slice/user-1000.slice/proxy.slice"

## some variables
port=12345
enable_tcp=true
enable_udp=true
enable_ipv4=true
enable_ipv6=true

## do not modify this if you don't known what you are doing
mark=2333
table=100
v2ray_so_mark=255

## cgroup things
# cgroup_mount_point=$(findmnt -t cgroup,cgroup2 -n -J|jq '.filesystems[0].target')
# cgroup_type=$(findmnt -t cgroup,cgroup2 -n -J|jq '.filesystems[0].fstype')
cgroup_mount_point="/sys/fs/cgroup"
cgroup_type="cgroup2"
cgroup_procs_file="cgroup.procs"

## parse parameter
for i in "$@"
do
case $i in
    stop)
        iptables -t mangle -F
        iptables -t mangle -X TPROXY_PRE
        iptables -t mangle -X TPROXY_OUT
        ip6tables -t mangle -F
        ip6tables -t mangle -X TPROXY_PRE
        ip6tables -t mangle -X TPROXY_OUT
        ip rule delete fwmark $mark lookup $table
        ip route flush table $table
        ip -6 rule delete fwmark $mark lookup $table
        ip -6 route flush table $table
        exit 0
        ;;
    --config=*)
        config=${i#*=}
        source $config
        shift
        ;;
    --help)
        print_help
        exit 0
        ;;
esac
done

## TODO cgroup need to exists before using in iptables since 5.6.5, maybe it's bug
test -d $cgroup_mount_point$proxy_cgroup || mkdir $cgroup_mount_point$proxy_cgroup || exit -1; 

## use TPROXY
#ipv4#
ip rule add fwmark $mark table $table
ip route add local default dev lo table $table
iptables -t mangle -N TPROXY_PRE
iptables -t mangle -A TPROXY_PRE -p udp -m mark --mark $mark -j TPROXY --on-ip 127.0.0.1 --on-port $port --tproxy-mark $mark
iptables -t mangle -A TPROXY_PRE -p tcp -m mark --mark $mark -j TPROXY --on-ip 127.0.0.1 --on-port $port --tproxy-mark $mark
iptables -t mangle -A PREROUTING -j TPROXY_PRE

iptables -t mangle -N TPROXY_OUT
iptables -t mangle -A TPROXY_OUT -p udp -o lo -j RETURN
iptables -t mangle -A TPROXY_OUT -p udp -m cgroup --path $proxy_cgroup -j MARK --set-mark $mark
iptables -t mangle -A TPROXY_OUT -p tcp -o lo -j RETURN
iptables -t mangle -A TPROXY_OUT -p tcp -m cgroup --path $proxy_cgroup -j MARK --set-mark $mark
iptables -t mangle -A OUTPUT -m mark ! --mark $v2ray_so_mark -j TPROXY_OUT

#ipv6#
ip -6 rule add fwmark $mark table $table
ip -6 route add local default dev lo table $table
ip6tables -t mangle -N TPROXY_PRE
ip6tables -t mangle -A TPROXY_PRE -p udp -m mark --mark $mark -j TPROXY --on-ip ::1 --on-port $port --tproxy-mark $mark
ip6tables -t mangle -A TPROXY_PRE -p tcp -m mark --mark $mark -j TPROXY --on-ip ::1 --on-port $port --tproxy-mark $mark
ip6tables -t mangle -A PREROUTING -j TPROXY_PRE

ip6tables -t mangle -N TPROXY_OUT
ip6tables -t mangle -A TPROXY_OUT -p udp -o lo -j RETURN
ip6tables -t mangle -A TPROXY_OUT -p udp -m cgroup --path $proxy_cgroup -j MARK --set-mark $mark
ip6tables -t mangle -A TPROXY_OUT -p tcp -o lo -j RETURN
ip6tables -t mangle -A TPROXY_OUT -p tcp -m cgroup --path $proxy_cgroup -j MARK --set-mark $mark
ip6tables -t mangle -A OUTPUT -m mark ! --mark $v2ray_so_mark -j TPROXY_OUT

## allow to disable, order is important
$enable_udp 	|| iptables  -t mangle -I TPROXY_OUT -p udp -j RETURN
$enable_udp 	|| ip6tables -t mangle -I TPROXY_OUT -p udp -j RETURN
$enable_tcp 	|| iptables  -t mangle -I TPROXY_OUT -p tcp -j RETURN
$enable_tcp 	|| ip6tables -t mangle -I TPROXY_OUT -p tcp -j RETURN
$enable_ipv4 	|| iptables  -t mangle -I TPROXY_OUT -j RETURN
$enable_ipv6 	|| ip6tables -t mangle -I TPROXY_OUT -j RETURN


## create proxy prefix command for easy use
# cat << 'DOC' > /usr/bin/cgproxy
# !/usr/bin/bash
# systemd-run -q --slice proxy.slice --scope --user $@
# DOC
# chmod a+x /usr/bin/cgproxy

## message for user
cat << DOC
proxied cgroup: $proxy_cgroup
DOC

## tproxy need Root or cap_net_admin capability
# setcap cap_net_admin+ep /usr/lib/v2ray/v2ray


#!/bin/bash
print_help(){
cat << 'DOC'
#############################################################################
# 
# 1. This script need cgroup v2 
# 
# 2. Listening port is expected to accept iptables TPROXY, while REDIRECT 
#    will not work in this script, because REDIRECT only support tcp/ipv4
# 
# 3. TPROXY need root or special capability whatever process is listening on port
#    v2ray as example: 
#       sudo setcap "cap_net_bind_service=+ep cap_net_admin=+ep" /usr/lib/v2ray/v2ray
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
cgroup_proxy="/proxy.slice"
cgroup_noproxy="/noproxy.slice"

# allow as gateway for local network
enable_gateway=true

## some variables
port=12345
enable_tcp=true
enable_udp=true
enable_ipv4=true
enable_ipv6=true
enable_dns=true

## do not modify this if you don't known what you are doing
table=100
mark_proxy=0x01
mark_noproxy=0xff
make_newin=0x02

## cgroup things
# cgroup_mount_point=$(findmnt -t cgroup,cgroup2 -n -J|jq '.filesystems[0].target')
# cgroup_type=$(findmnt -t cgroup,cgroup2 -n -J|jq '.filesystems[0].fstype')
cgroup_mount_point=$(findmnt -t cgroup2 -n |cut -d' ' -f 1)
cgroup_type="cgroup2"
cgroup_procs_file="cgroup.procs"

## parse parameter
for i in "$@"
do
case $i in
    stop)
        iptables -t nat -F
        iptables -t mangle -F
        iptables -t mangle -X TPROXY_PRE
        iptables -t mangle -X TPROXY_OUT
        ip6tables -t mangle -F
        ip6tables -t mangle -X TPROXY_PRE
        ip6tables -t mangle -X TPROXY_OUT
        ip rule delete fwmark $mark_proxy lookup $table
        ip route flush table $table
        ip -6 rule delete fwmark $mark_proxy lookup $table
        ip -6 route flush table $table
        iptables -t nat -A OUTPUT -F
        ip6tables -t nat -A OUTPUT -F
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
test -d $cgroup_mount_point$cgroup_proxy    || mkdir $cgroup_mount_point$cgroup_proxy   || exit -1; 
test -d $cgroup_mount_point$cgroup_noproxy  || mkdir $cgroup_mount_point$cgroup_noproxy || exit -1; 

## use TPROXY
#ipv4#
ip rule add fwmark $mark_proxy table $table
ip route add local default dev lo table $table
iptables -t mangle -N TPROXY_PRE
iptables -t mangle -A TPROXY_PRE -m addrtype --dst-type LOCAL -j RETURN
iptables -t mangle -A TPROXY_PRE -m pkttype --pkt-type broadcast -j RETURN
iptables -t mangle -A TPROXY_PRE -m pkttype --pkt-type multicast -j RETURN
iptables -t mangle -A TPROXY_PRE -p tcp -j TPROXY --on-ip 127.0.0.1 --on-port $port --tproxy-mark $mark_proxy
iptables -t mangle -A TPROXY_PRE -p udp -j TPROXY --on-ip 127.0.0.1 --on-port $port --tproxy-mark $mark_proxy
iptables -t mangle -A PREROUTING -j TPROXY_PRE
iptables -t mangle -A PREROUTING -m addrtype --dst-type LOCAL -m conntrack --ctstate NEW -j CONNMARK --set-mark $make_newin

iptables -t mangle -N TPROXY_OUT
iptables -t mangle -A TPROXY_OUT -o lo -j RETURN
iptables -t mangle -A TPROXY_OUT -p icmp -j RETURN
iptables -t mangle -A TPROXY_OUT -m connmark --mark  $make_newin -j RETURN # return incoming connection directly
iptables -t mangle -A TPROXY_OUT  -m addrtype ! --src-type LOCAL -m addrtype ! --dst-type LOCAL -j RETURN
iptables -t mangle -A TPROXY_OUT -m mark --mark $mark_noproxy -j RETURN
iptables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_noproxy -j RETURN
iptables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_proxy -j MARK --set-mark $mark_proxy
iptables -t mangle -A OUTPUT -j TPROXY_OUT

#ipv6#
ip -6 rule add fwmark $mark_proxy table $table
ip -6 route add local default dev lo table $table
ip6tables -t mangle -N TPROXY_PRE
ip6tables -t mangle -A TPROXY_PRE -m addrtype --dst-type LOCAL -j RETURN
ip6tables -t mangle -A TPROXY_PRE -m pkttype --pkt-type broadcast -j RETURN
ip6tables -t mangle -A TPROXY_PRE -m pkttype --pkt-type multicast -j RETURN
ip6tables -t mangle -A TPROXY_PRE -p tcp -j TPROXY --on-ip ::1 --on-port $port --tproxy-mark $mark_proxy
ip6tables -t mangle -A TPROXY_PRE -p udp -j TPROXY --on-ip ::1 --on-port $port --tproxy-mark $mark_proxy
ip6tables -t mangle -A PREROUTING -j TPROXY_PRE
ip6tables -t mangle -A PREROUTING -m addrtype --dst-type LOCAL -m conntrack --ctstate NEW -j CONNMARK --set-mark $make_newin

ip6tables -t mangle -N TPROXY_OUT
ip6tables -t mangle -A TPROXY_OUT -o lo -j RETURN
ip6tables -t mangle -A TPROXY_OUT -p icmp -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m connmark --mark  $make_newin -j RETURN # return incoming connection directly
ip6tables -t mangle -A TPROXY_OUT  -m addrtype ! --src-type LOCAL -m addrtype ! --dst-type LOCAL -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m mark --mark $mark_noproxy -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_noproxy -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_proxy -j MARK --set-mark $mark_proxy
ip6tables -t mangle -A OUTPUT -j TPROXY_OUT

## allow to disable, order is important
$enable_dns    	|| iptables  -t mangle -I TPROXY_OUT -p udp --dport 53 -j RETURN
$enable_dns    	|| ip6tables -t mangle -I TPROXY_OUT -p udp --dport 53 -j RETURN
$enable_udp 	|| iptables  -t mangle -I TPROXY_OUT -p udp -j RETURN
$enable_udp 	|| ip6tables -t mangle -I TPROXY_OUT -p udp -j RETURN
$enable_tcp 	|| iptables  -t mangle -I TPROXY_OUT -p tcp -j RETURN
$enable_tcp 	|| ip6tables -t mangle -I TPROXY_OUT -p tcp -j RETURN
$enable_ipv4 	|| iptables  -t mangle -I TPROXY_OUT -j RETURN
$enable_ipv6 	|| ip6tables -t mangle -I TPROXY_OUT -j RETURN

if $enable_gateway; then
$enable_dns    	|| iptables  -t mangle -I TPROXY_PRE -p udp --dport 53 -j RETURN
$enable_dns    	|| ip6tables -t mangle -I TPROXY_PRE -p udp --dport 53 -j RETURN
$enable_udp 	|| iptables  -t mangle -I TPROXY_PRE -p udp -j RETURN
$enable_udp 	|| ip6tables -t mangle -I TPROXY_PRE -p udp -j RETURN
$enable_tcp 	|| iptables  -t mangle -I TPROXY_PRE -p tcp -j RETURN
$enable_tcp 	|| ip6tables -t mangle -I TPROXY_PRE -p tcp -j RETURN
$enable_ipv4 	|| iptables  -t mangle -I TPROXY_PRE -j RETURN
$enable_ipv6 	|| ip6tables -t mangle -I TPROXY_PRE -j RETURN
fi


## message for user
cat << DOC
proxied cgroup: $cgroup_proxy
DOC


if $enable_gateway; then
    iptables  -t nat -A POSTROUTING -m addrtype ! --src-type LOCAL -j MASQUERADE
    ip6tables -t nat -A POSTROUTING -m addrtype ! --src-type LOCAL -j MASQUERADE
    sysctl -w net.ipv4.ip_forward=1
    sysctl -w net.ipv6.conf.all.forwarding=1
    echo "gateway enabled"
fi

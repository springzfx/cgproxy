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
enable_gateway=false

## some variables
port=12345

## some options
enable_dns=true
enable_tcp=true
enable_udp=true
enable_ipv4=true
enable_ipv6=true

## do not modify this if you don't known what you are doing
table=100
fwmark=0x01
make_newin=0x02

## cgroup things
cgroup_mount_point=$(findmnt -t cgroup2 -n -o TARGET)
cgroup_type="cgroup2"
cgroup_procs_file="cgroup.procs"

## parse parameter
for i in "$@"
do
case $i in
    stop)
        iptables -t mangle -D PREROUTING -j TPROXY_PRE
        iptables -t mangle -D OUTPUT -j TPROXY_OUT
        iptables -t mangle -F TPROXY_PRE
        iptables -t mangle -F TPROXY_OUT
        iptables -t mangle -F TPROXY_ENT
        iptables -t mangle -X TPROXY_PRE
        iptables -t mangle -X TPROXY_OUT
        iptables -t mangle -X TPROXY_ENT
        ip6tables -t mangle -D PREROUTING -j TPROXY_PRE
        ip6tables -t mangle -D OUTPUT -j TPROXY_OUT
        ip6tables -t mangle -F TPROXY_PRE
        ip6tables -t mangle -F TPROXY_OUT
        ip6tables -t mangle -F TPROXY_ENT
        ip6tables -t mangle -X TPROXY_PRE
        ip6tables -t mangle -X TPROXY_OUT
        ip6tables -t mangle -X TPROXY_ENT
        ip rule delete fwmark $fwmark lookup $table
        ip route flush table $table
        ip -6 rule delete fwmark $fwmark lookup $table
        ip -6 route flush table $table
        ## may not exist, just ignore, and tracking their existence is not reliable
        iptables -t nat -D POSTROUTING -m owner ! --socket-exists -j MASQUERADE &> /dev/null
        ip6tables -t nat -D POSTROUTING -m owner ! --socket-exists -j MASQUERADE &> /dev/null
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
ip rule add fwmark $fwmark table $table
ip route add local default dev lo table $table
iptables -t mangle -N TPROXY_ENT
iptables -t mangle -A TPROXY_ENT -p tcp -j TPROXY --on-ip localhost --on-port $port --tproxy-mark $fwmark
iptables -t mangle -A TPROXY_ENT -p udp -j TPROXY --on-ip localhost --on-port $port --tproxy-mark $fwmark

iptables -t mangle -N TPROXY_PRE
iptables -t mangle -A TPROXY_PRE -m socket --transparent -j MARK --set-mark $fwmark
iptables -t mangle -A TPROXY_PRE -m socket --transparent -j RETURN
iptables -t mangle -A TPROXY_PRE -p icmp -j RETURN
iptables -t mangle -A TPROXY_PRE -p udp --dport 53 -j TPROXY_ENT
iptables -t mangle -A TPROXY_PRE -p tcp --dport 53 -j TPROXY_ENT
iptables -t mangle -A TPROXY_PRE -m addrtype --dst-type LOCAL -j RETURN
iptables -t mangle -A TPROXY_PRE -m addrtype ! --dst-type UNICAST -j RETURN
iptables -t mangle -A TPROXY_PRE -j TPROXY_ENT
iptables -t mangle -A PREROUTING -j TPROXY_PRE

iptables -t mangle -N TPROXY_OUT
iptables -t mangle -A TPROXY_OUT -p icmp -j RETURN
iptables -t mangle -A TPROXY_OUT -m connmark --mark  $make_newin -j RETURN
iptables -t mangle -A TPROXY_OUT -m addrtype --dst-type LOCAL -j RETURN
iptables -t mangle -A TPROXY_OUT -m addrtype ! --dst-type UNICAST -j RETURN
iptables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_noproxy -j RETURN
iptables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_proxy -j MARK --set-mark $fwmark
iptables -t mangle -A OUTPUT -j TPROXY_OUT

#ipv6#
ip -6 rule add fwmark $fwmark table $table
ip -6 route add local default dev lo table $table
ip6tables -t mangle -N TPROXY_ENT
ip6tables -t mangle -A TPROXY_ENT -p tcp -j TPROXY --on-ip localhost --on-port $port --tproxy-mark $fwmark
ip6tables -t mangle -A TPROXY_ENT -p udp -j TPROXY --on-ip localhost --on-port $port --tproxy-mark $fwmark

ip6tables -t mangle -N TPROXY_PRE
ip6tables -t mangle -A TPROXY_PRE -m socket --transparent -j MARK --set-mark $fwmark
ip6tables -t mangle -A TPROXY_PRE -m socket --transparent -j RETURN
ip6tables -t mangle -A TPROXY_PRE -p icmpv6 -j RETURN
ip6tables -t mangle -A TPROXY_PRE -p udp --dport 53 -j TPROXY_ENT
ip6tables -t mangle -A TPROXY_PRE -p tcp --dport 53 -j TPROXY_ENT
ip6tables -t mangle -A TPROXY_PRE -m addrtype --dst-type LOCAL -j RETURN
ip6tables -t mangle -A TPROXY_PRE -m addrtype ! --dst-type UNICAST -j RETURN
ip6tables -t mangle -A TPROXY_PRE -j TPROXY_ENT
ip6tables -t mangle -A PREROUTING -j TPROXY_PRE

ip6tables -t mangle -N TPROXY_OUT
ip6tables -t mangle -A TPROXY_OUT -p icmpv6 -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m connmark --mark  $make_newin -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m addrtype --dst-type LOCAL -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m addrtype ! --dst-type UNICAST -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_noproxy -j RETURN
ip6tables -t mangle -A TPROXY_OUT -m cgroup --path $cgroup_proxy -j MARK --set-mark $fwmark
ip6tables -t mangle -A OUTPUT -j TPROXY_OUT

## allow to disable, order is important
$enable_dns     || iptables  -t mangle -I TPROXY_OUT -p udp --dport 53 -j RETURN
$enable_dns     || ip6tables -t mangle -I TPROXY_OUT -p udp --dport 53 -j RETURN
$enable_udp     || iptables  -t mangle -I TPROXY_OUT -p udp -j RETURN
$enable_udp     || ip6tables -t mangle -I TPROXY_OUT -p udp -j RETURN
$enable_tcp     || iptables  -t mangle -I TPROXY_OUT -p tcp -j RETURN
$enable_tcp     || ip6tables -t mangle -I TPROXY_OUT -p tcp -j RETURN
$enable_ipv4    || iptables  -t mangle -I TPROXY_OUT -j RETURN
$enable_ipv6    || ip6tables -t mangle -I TPROXY_OUT -j RETURN

if $enable_gateway; then
$enable_dns     || iptables  -t mangle -I TPROXY_PRE -p udp --dport 53 -j RETURN
$enable_dns     || ip6tables -t mangle -I TPROXY_PRE -p udp --dport 53 -j RETURN
$enable_udp     || iptables  -t mangle -I TPROXY_PRE -p udp -j RETURN
$enable_udp     || ip6tables -t mangle -I TPROXY_PRE -p udp -j RETURN
$enable_tcp     || iptables  -t mangle -I TPROXY_PRE -p tcp -j RETURN
$enable_tcp     || ip6tables -t mangle -I TPROXY_PRE -p tcp -j RETURN
$enable_ipv4    || iptables  -t mangle -I TPROXY_PRE -j RETURN
$enable_ipv6    || ip6tables -t mangle -I TPROXY_PRE -j RETURN
fi

## do not handle local device connection through tproxy if gateway is not enabled
$enable_gateway || iptables  -t mangle -I TPROXY_PRE -m addrtype ! --src-type LOCAL -j RETURN
$enable_gateway || ip6tables -t mangle -I TPROXY_PRE -m addrtype ! --src-type LOCAL -j RETURN

## make sure following rules are the first in chain TPROXY_PRE to mark new incoming connection or gateway proxy connection
## so must put at last to insert first
iptables  -t mangle -I TPROXY_PRE -m addrtype ! --src-type LOCAL -m conntrack --ctstate NEW -j CONNMARK --set-mark $make_newin
ip6tables -t mangle -I TPROXY_PRE -m addrtype ! --src-type LOCAL -m conntrack --ctstate NEW -j CONNMARK --set-mark $make_newin

## message for user
cat << DOC
noproxy cgroup: $cgroup_noproxy
proxied cgroup: $cgroup_proxy
DOC


if $enable_gateway; then
    iptables  -t nat -A POSTROUTING -m owner ! --socket-exists -j MASQUERADE
    ip6tables -t nat -A POSTROUTING -m owner ! --socket-exists -j MASQUERADE
    sysctl -w net.ipv4.ip_forward=1
    sysctl -w net.ipv6.conf.all.forwarding=1
    echo "gateway enabled"
fi

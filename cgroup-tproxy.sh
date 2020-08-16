#!/bin/bash
### This script will proxy/noproxy anything running in specific cgroup
### need cgroup2 support, and iptables cgroup2 path match support
### 
### script usage:
###     cgroup-tproxy.sh [--help|--config|stop]
### options:
###     --config=FILE   load config from file
###            --help   show help info
###              stop   clean then stop. Variables may change when stopping, which should be avoid
###                     so always stop first in the last context before start new context
###
### available variables with default value:
###     cgroup_noproxy="/noproxy.slice" 
###     cgroup_proxy="/proxy.slice" 
###     port=12345
###     enable_dns=true
###     enable_udp=true
###     enable_tcp=true
###     enable_ipv4=true
###     enable_ipv6=true
###     enable_gateway=false
###     table=10007
###     fwmark=0x9973
###     cgroup_mount_point=$(findmnt -t cgroup2 -n -o TARGET | head -n 1)
###
### semicolon to seperate multi cgroup:
###     cgroup_noproxy="/noproxy1.slice:/noproxy2.slice"
###     cgroup_proxy="/proxy1.slice:/proxy2.slice"

print_help(){
    sed -rn 's/^### ?//;T;p' "$0"
}

## check root
[ ! $(id -u) -eq 0 ] && { >&2 echo "iptables: need root to modify iptables";exit -1; }

## any process in this cgroup will be proxied
if [ -z ${cgroup_proxy+x} ]; then  
    cgroup_proxy="/proxy.slice"
else
    IFS=':' read -r -a cgroup_proxy     <<< "$cgroup_proxy"
fi

## any process in this cgroup will not be proxied
if [ -z ${cgroup_noproxy+x} ]; then  
    cgroup_noproxy="/noproxy.slice"
else
    IFS=':' read -r -a cgroup_noproxy   <<< "$cgroup_noproxy"
fi

## tproxy listening port
[ -z ${port+x} ] && port=12345

## controll options
[ -z ${enable_dns+x} ]  && enable_dns=true
[ -z ${enable_udp+x} ]  && enable_udp=true
[ -z ${enable_tcp+x} ]  && enable_tcp=true
[ -z ${enable_ipv4+x} ] && enable_ipv4=true
[ -z ${enable_ipv6+x} ] && enable_ipv6=true
[ -z ${enable_gateway+x} ] && enable_gateway=false

## mark/route things
[ -z ${table+x} ]           && table=10007
[ -z ${fwmark+x} ]          && fwmark=0x9973
[ -z ${table_reroute+x} ]   && table_reroute=$table
[ -z ${table_tproxy+x} ]    && table_tproxy=$table
[ -z ${fwmark_reroute+x} ]  && fwmark_reroute=$fwmark
[ -z ${fwmark_tproxy+x} ]   && fwmark_tproxy=$fwmark

## cgroup mount point things
[ -z ${cgroup_mount_point+x} ] && cgroup_mount_point=$(findmnt -t cgroup2 -n -o TARGET | head -n 1)


stop(){
    iptables -w 60 -t mangle -L TPROXY_ENT &> /dev/null || return
    echo "iptables: cleaning tproxy iptables"

    iptables -w 60 -t mangle -D PREROUTING -j TPROXY_PRE
    iptables -w 60 -t mangle -D OUTPUT -j TPROXY_OUT

    iptables -w 60 -t mangle -F TPROXY_PRE
    iptables -w 60 -t mangle -F TPROXY_ENT
    iptables -w 60 -t mangle -F TPROXY_OUT
    iptables -w 60 -t mangle -F TPROXY_MARK

    iptables -w 60 -t mangle -X TPROXY_PRE
    iptables -w 60 -t mangle -X TPROXY_ENT
    iptables -w 60 -t mangle -X TPROXY_OUT
    iptables -w 60 -t mangle -X TPROXY_MARK

    ip rule delete fwmark $fwmark_tproxy lookup $table_tproxy
    ip rule delete fwmark $fwmark_reroute lookup $table_reroute &> /dev/null
    ip route flush table $table_tproxy
    ip route flush table $table_reroute &> /dev/null

    ip6tables -w 60 -t mangle -D PREROUTING -j TPROXY_PRE
    ip6tables -w 60 -t mangle -D OUTPUT -j TPROXY_OUT

    ip6tables -w 60 -t mangle -F TPROXY_PRE
    ip6tables -w 60 -t mangle -F TPROXY_OUT
    ip6tables -w 60 -t mangle -F TPROXY_ENT
    ip6tables -w 60 -t mangle -F TPROXY_MARK

    ip6tables -w 60 -t mangle -X TPROXY_PRE
    ip6tables -w 60 -t mangle -X TPROXY_OUT
    ip6tables -w 60 -t mangle -X TPROXY_ENT
    ip6tables -w 60 -t mangle -X TPROXY_MARK

    ip -6 rule delete fwmark $fwmark_tproxy lookup $table_tproxy
    ip -6 rule delete fwmark $fwmark_reroute lookup $table_reroute &> /dev/null
    ip -6 route flush table $table_tproxy
    ip -6 route flush table $table_reroute &> /dev/null

    ## may not exist, just ignore, and tracking their existence is not reliable
    iptables -w 60 -t nat -D POSTROUTING -m owner ! --socket-exists -j MASQUERADE &> /dev/null
    ip6tables -w 60 -t nat -D POSTROUTING -m owner ! --socket-exists -s fc00::/7 -j MASQUERADE &> /dev/null

    ## unmount cgroup2
    [ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" = "cgroup2" ] && umount $cgroup_mount_point
}

## parse parameter
for i in "$@"
do
case $i in
    stop)
        stop
        exit 0
        ;;
    --config=*)
        config=${i#*=}
        source $config
        ;;
    --help)
        print_help
        exit 0
        ;;
    *)
        print_help
        exit 0
        ;;
esac
done


## check cgroup_mount_point, create and mount if necessary
[ -z $cgroup_mount_point ] && { >&2 echo "iptables: no cgroup2 mount point available"; exit -1; }
[ ! -d $cgroup_mount_point ] && mkdir -p $cgroup_mount_point
[ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" != "cgroup2" ] && mount -t cgroup2 none $cgroup_mount_point
[ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" != "cgroup2" ] && { >&2 echo "iptables: mount $cgroup_mount_point failed"; exit -1; }

## only create the first one in arrary
test -d $cgroup_mount_point$cgroup_proxy    || mkdir $cgroup_mount_point$cgroup_proxy   || exit -1; 
test -d $cgroup_mount_point$cgroup_noproxy  || mkdir $cgroup_mount_point$cgroup_noproxy || exit -1; 

## filter cgroup that not exist
_cgroup_noproxy=()
for cg in ${cgroup_noproxy[@]}; do
    test -d $cgroup_mount_point$cg && _cgroup_noproxy+=($cg) || { >&2 echo "iptables: $cg not exist, ignore";}
done
unset cgroup_noproxy && cgroup_noproxy=${_cgroup_noproxy[@]}

## filter cgroup that not exist
_cgroup_proxy=()
for cg in ${cgroup_proxy[@]}; do
    test -d $cgroup_mount_point$cg && _cgroup_proxy+=($cg) || { >&2 echo "iptables: $cg not exist, ignore";}
done
unset cgroup_proxy && cgroup_proxy=${_cgroup_proxy[@]}


## ipv4 #########################################################################
echo "iptables: applying tproxy iptables"
## mangle prerouting
ip rule add fwmark $fwmark_tproxy table $table_tproxy
ip route add local default dev lo table $table_tproxy
# core
iptables -w 60 -t mangle -N TPROXY_ENT
iptables -w 60 -t mangle -A TPROXY_ENT -m socket -j MARK --set-mark $fwmark_tproxy
iptables -w 60 -t mangle -A TPROXY_ENT -m socket -j ACCEPT
iptables -w 60 -t mangle -A TPROXY_ENT -p tcp -j TPROXY --on-ip 127.0.0.1 --on-port $port --tproxy-mark $fwmark_tproxy
iptables -w 60 -t mangle -A TPROXY_ENT -p udp -j TPROXY --on-ip 127.0.0.1 --on-port $port --tproxy-mark $fwmark_tproxy
# filter
iptables -w 60 -t mangle -N TPROXY_PRE
iptables -w 60 -t mangle -A TPROXY_PRE -m addrtype --dst-type LOCAL -j RETURN
iptables -w 60 -t mangle -A TPROXY_PRE -m addrtype ! --dst-type UNICAST -j RETURN
$enable_gateway  || iptables -w 60 -t mangle -A TPROXY_PRE -m addrtype ! --src-type LOCAL -j RETURN
$enable_dns && iptables -w 60 -t mangle -A TPROXY_PRE -p udp --dport 53 -j TPROXY_ENT
$enable_udp && iptables -w 60 -t mangle -A TPROXY_PRE -p udp -j TPROXY_ENT
$enable_tcp && iptables -w 60 -t mangle -A TPROXY_PRE -p tcp -j TPROXY_ENT
# hook
iptables -w 60 -t mangle -A PREROUTING -j TPROXY_PRE

## mangle output
if [ $fwmark_reroute != $fwmark_tproxy ]; then
ip rule add fwmark $fwmark_reroute table $table_reroute
ip route add local default dev lo table $table_reroute
fi
# filter
iptables -w 60 -t mangle -N TPROXY_MARK
iptables -w 60 -t mangle -A TPROXY_MARK -m addrtype ! --dst-type UNICAST -j RETURN
$enable_dns && iptables -w 60 -t mangle -A TPROXY_MARK -p udp --dport 53 -j MARK --set-mark $fwmark_reroute
$enable_udp && iptables -w 60 -t mangle -A TPROXY_MARK -p udp -j MARK --set-mark $fwmark_reroute
$enable_tcp && iptables -w 60 -t mangle -A TPROXY_MARK -p tcp -j MARK --set-mark $fwmark_reroute
# cgroup
iptables -w 60 -t mangle -N TPROXY_OUT
iptables -w 60 -t mangle -A TPROXY_OUT -m conntrack --ctdir REPLY -j RETURN
for cg in ${cgroup_noproxy[@]}; do
iptables -w 60 -t mangle -A TPROXY_OUT -m cgroup --path $cg -j RETURN
done
for cg in ${cgroup_proxy[@]}; do
iptables -w 60 -t mangle -A TPROXY_OUT -m cgroup --path $cg -j TPROXY_MARK
done
# hook
$enable_ipv4 && iptables -w 60 -t mangle -A OUTPUT -j TPROXY_OUT

## ipv6 #########################################################################
## mangle prerouting
ip -6 rule add fwmark $fwmark_tproxy table $table_tproxy
ip -6 route add local default dev lo table $table_tproxy
# core
ip6tables -w 60 -t mangle -N TPROXY_ENT
ip6tables -w 60 -t mangle -A TPROXY_ENT -m socket -j MARK --set-mark $fwmark_tproxy
ip6tables -w 60 -t mangle -A TPROXY_ENT -m socket -j ACCEPT
ip6tables -w 60 -t mangle -A TPROXY_ENT -p tcp -j TPROXY --on-ip ::1 --on-port $port --tproxy-mark $fwmark_tproxy
ip6tables -w 60 -t mangle -A TPROXY_ENT -p udp -j TPROXY --on-ip ::1 --on-port $port --tproxy-mark $fwmark_tproxy
# filter
ip6tables -w 60 -t mangle -N TPROXY_PRE
ip6tables -w 60 -t mangle -A TPROXY_PRE -m addrtype --dst-type LOCAL -j RETURN
ip6tables -w 60 -t mangle -A TPROXY_PRE -m addrtype ! --dst-type UNICAST -j RETURN
$enable_gateway  || ip6tables -w 60 -t mangle -A TPROXY_PRE -m addrtype ! --src-type LOCAL -j RETURN
$enable_dns && ip6tables -w 60 -t mangle -A TPROXY_PRE -p udp --dport 53 -j TPROXY_ENT
$enable_udp && ip6tables -w 60 -t mangle -A TPROXY_PRE -p udp -j TPROXY_ENT
$enable_tcp && ip6tables -w 60 -t mangle -A TPROXY_PRE -p tcp -j TPROXY_ENT
# hook
ip6tables -w 60 -t mangle -A PREROUTING -j TPROXY_PRE

## mangle output
if [ $fwmark_reroute != $fwmark_tproxy ]; then
ip -6 rule add fwmark $fwmark_reroute table $table_reroute
ip -6 route add local default dev lo table $table_reroute
fi
# filter
ip6tables -w 60 -t mangle -N TPROXY_MARK
ip6tables -w 60 -t mangle -A TPROXY_MARK -m addrtype ! --dst-type UNICAST -j RETURN
$enable_dns && ip6tables -w 60 -t mangle -A TPROXY_MARK -p udp --dport 53 -j MARK --set-mark $fwmark_reroute
$enable_udp && ip6tables -w 60 -t mangle -A TPROXY_MARK -p udp -j MARK --set-mark $fwmark_reroute
$enable_tcp && ip6tables -w 60 -t mangle -A TPROXY_MARK -p tcp -j MARK --set-mark $fwmark_reroute
# cgroup
ip6tables -w 60 -t mangle -N TPROXY_OUT
ip6tables -w 60 -t mangle -A TPROXY_OUT -m conntrack --ctdir REPLY -j RETURN
for cg in ${cgroup_noproxy[@]}; do
ip6tables -w 60 -t mangle -A TPROXY_OUT -m cgroup --path $cg -j RETURN
done
for cg in ${cgroup_proxy[@]}; do
ip6tables -w 60 -t mangle -A TPROXY_OUT -m cgroup --path $cg -j TPROXY_MARK
done
# hook
$enable_ipv6 && ip6tables -w 60 -t mangle -A OUTPUT -j TPROXY_OUT

## forward #######################################################################
if $enable_gateway; then
    iptables  -t nat -A POSTROUTING -m owner ! --socket-exists -j MASQUERADE
    ip6tables -w 60 -t nat -A POSTROUTING -m owner ! --socket-exists -s fc00::/7 -j MASQUERADE # only masquerade ipv6 private address
    sysctl -w net.ipv4.ip_forward=1
    sysctl -w net.ipv6.conf.all.forwarding=1
    echo "iptables: gateway enabled"
fi

## message for user
cat << DOC
iptables: noproxy cgroup: ${cgroup_noproxy[@]}
iptables: proxied cgroup: ${cgroup_proxy[@]}
DOC

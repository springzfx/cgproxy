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
[ ! $(id -u) -eq 0 ] && { >&2 echo "iptables/nftables: need root to modify iptables/nftables";exit -1; }

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
[ -z ${enable_nftables+x} ] && enable_nftables=false

## cgroup mount point things
[ -z ${cgroup_mount_point+x} ] && cgroup_mount_point=$(findmnt -t cgroup2 -n -o TARGET | head -n 1)

stop=false
## parse parameter
for i in "$@"
do
case $i in
    stop)
        stop=true
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

$enable_nftables && backend=nftables || backend=iptables
source @CMAKE_INSTALL_FULL_DATADIR@/cgproxy/scripts/$backend.sh

$stop && { stop;exit 0; }
## check cgroup_mount_point, create and mount if necessary
[ -z $cgroup_mount_point ] && { >&2 echo "$backend: no cgroup2 mount point available"; exit -1; }
[ ! -d $cgroup_mount_point ] && mkdir -p $cgroup_mount_point
[ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" != "cgroup2" ] && mount -t cgroup2 none $cgroup_mount_point
[ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" != "cgroup2" ] && { >&2 echo "$backend: mount $cgroup_mount_point failed"; exit -1; }

## only create the first one in arrary
test -d $cgroup_mount_point$cgroup_proxy    || mkdir $cgroup_mount_point$cgroup_proxy   || exit -1; 
test -d $cgroup_mount_point$cgroup_noproxy  || mkdir $cgroup_mount_point$cgroup_noproxy || exit -1; 

## filter cgroup that not exist
_cgroup_noproxy=()
for cg in ${cgroup_noproxy[@]}; do
    test -d $cgroup_mount_point$cg && _cgroup_noproxy+=($cg) || { >&2 echo "$backend: $cg not exist, ignore";}
done
unset cgroup_noproxy && cgroup_noproxy=${_cgroup_noproxy[@]}

## filter cgroup that not exist
_cgroup_proxy=()
for cg in ${cgroup_proxy[@]}; do
    test -d $cgroup_mount_point$cg && _cgroup_proxy+=($cg) || { >&2 echo "$backend: $cg not exist, ignore";}
done
unset cgroup_proxy && cgroup_proxy=${_cgroup_proxy[@]}

start

## message for user
cat << DOC
$backend: noproxy cgroup: ${cgroup_noproxy[@]}
$backend: proxied cgroup: ${cgroup_proxy[@]}
DOC

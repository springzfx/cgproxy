#!/bin/bash
### This script will proxy/noproxy anything running in specific cgroup
### need cgroup2 support, and iptables cgroup2 path match support
### 
### script usage:
###     cgtproxy.sh [--help|--config|setup|teardown]
### options:
###     --config=FILE   load config from file
###            --help   show help info
###             setup   setup
###          teardown   clean then stop. Variables may change when stopping, which should be avoid
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
###     packetfilter_backend=iptables
###
### semicolon to seperate multi cgroup:
###     cgroup_noproxy="/noproxy1.slice:/noproxy2.slice"
###     cgroup_proxy="/proxy1.slice:/proxy2.slice"

self_name=${0##*/}

print_help(){
    sed -rn 's/^### ?//;T;p' "$0"
}

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
: ${port:=12345}

## controll options
: ${enable_dns:=true}
: ${enable_udp:=true}
: ${enable_tcp:=true}
: ${enable_ipv4:=true}
: ${enable_ipv6:=true}
: ${enable_gateway:=false}

## mark/route things
: ${table:=10007}
: ${fwmark:=0x9973}
: ${table_reroute:=$table}
: ${table_tproxy:=$table}
: ${fwmark_reroute:=$fwmark}
: ${fwmark_tproxy:=$fwmark}

## cgroup mount point things
: ${cgroup_mount_point:=$(findmnt -t cgroup2 -n -o TARGET | head -n 1)}

## packet filter backend
: ${packetfilter_backend:=iptables}



setup_cgroup(){
    ## check cgroup_mount_point, create and mount if necessary
    [ -z $cgroup_mount_point ] && { >&2 echo "${self_name}: no cgroup2 mount point available"; exit -1; }
    [ ! -d $cgroup_mount_point ] && mkdir -p $cgroup_mount_point
    [ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" != "cgroup2" ] && mount -t cgroup2 none $cgroup_mount_point
    [ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" != "cgroup2" ] && { >&2 echo "${self_name}: mount $cgroup_mount_point failed"; exit -1; }

    ## only create the first one in arrary
    test -d $cgroup_mount_point$cgroup_proxy    || mkdir $cgroup_mount_point$cgroup_proxy   || exit -1; 
    test -d $cgroup_mount_point$cgroup_noproxy  || mkdir $cgroup_mount_point$cgroup_noproxy || exit -1; 

    ## filter cgroup that not exist
    _cgroup_noproxy=()
    for cg in ${cgroup_noproxy[@]}; do
        test -d $cgroup_mount_point$cg && _cgroup_noproxy+=($cg) || { >&2 echo "${self_name}: $cg not exist, ignore";}
    done
    unset cgroup_noproxy && cgroup_noproxy=${_cgroup_noproxy[@]}

    ## filter cgroup that not exist
    _cgroup_proxy=()
    for cg in ${cgroup_proxy[@]}; do
        test -d $cgroup_mount_point$cg && _cgroup_proxy+=($cg) || { >&2 echo "${self_name}: $cg not exist, ignore";}
    done
    unset cgroup_proxy && cgroup_proxy=${_cgroup_proxy[@]}
}



case "${packetfilter_backend}" in
    nftables)
        source ${0%/*}/nft-tproxy.sh
    ;;
    iptables)
        source ${0%/*}/xt-tproxy.sh
    ;;
    *)
        >&2 echo "${self_name}: expected: iptables/nftables, got: \"${packetfilter_backend}\""; exit 1
    ;;
esac

setup(){
    setup_cgroup
    echo "${self_name}: applying ${packetfilter_backend} tproxy ruleset"

    ## mangle prerouting
    ip rule add fwmark $fwmark_tproxy table $table_tproxy
    ip route add local default dev lo table $table_tproxy
    ip -6 rule add fwmark $fwmark_tproxy table $table_tproxy
    ip -6 route add local default dev lo table $table_tproxy
    load_base_ruleset

    ## mangle output
    if [ $fwmark_reroute != $fwmark_tproxy ]; then
        ip rule add fwmark $fwmark_reroute table $table_reroute
        ip route add local default dev lo table $table_reroute
        ip -6 rule add fwmark $fwmark_reroute table $table_reroute
        ip -6 route add local default dev lo table $table_reroute
    fi
    load_local_ruleset

    if $enable_gateway; then
        # Not all packet filters currently have the full functionality of xt.
        load_gateway_ruleset && {
            sysctl -w net.ipv4.ip_forward=1
            sysctl -w net.ipv6.conf.all.forwarding=1
            echo "${self_name}: gateway enabled"
        }
    fi

    ## message for user
    cat << DOC
${self_name}: noproxy cgroup: ${cgroup_noproxy[@]}
${self_name}: proxied cgroup: ${cgroup_proxy[@]}
DOC
}

teardown(){
    unload_ruleset
    ## unmount cgroup2
    [ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" = "cgroup2" ] && umount $cgroup_mount_point
}



## parse parameter
for i in "$@"; do
case $i in
    teardown)
        ## check root
        [ ! $(id -u) -eq 0 ] && { >&2 echo "${self_name}: Requires root to control the package filter."; exit -1; }
        #teardown
        unload_ruleset
        exit 0
    ;;
    setup)
        ## check root
        [ ! $(id -u) -eq 0 ] && { >&2 echo "${self_name}: Requires root to control the package filter."; exit -1; }
        setup
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
        exit 1
    ;;
esac
done

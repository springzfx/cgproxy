stop() {
    nft list table inet cgproxy &> /dev/null || return
    echo "nftables: cleaning tproxy rules"
    
    nft delete table inet cgproxy

    ip rule delete fwmark $fwmark_tproxy lookup $table_tproxy
    ip rule delete fwmark $fwmark_reroute lookup $table_reroute &> /dev/null
    ip route flush table $table_tproxy
    ip route flush table $table_reroute &> /dev/null

    ip -6 rule delete fwmark $fwmark_tproxy lookup $table_tproxy
    ip -6 rule delete fwmark $fwmark_reroute lookup $table_reroute &> /dev/null
    ip -6 route flush table $table_tproxy
    ip -6 route flush table $table_reroute &> /dev/null

    ## unmount cgroup2
    [ "$(findmnt -M $cgroup_mount_point -n -o FSTYPE)" = "cgroup2" ] && umount $cgroup_mount_point
}

start() {
    ## nft ##########################################################################
    echo "nftables: applying tproxy nft rules"

    local _l4proto=()
    $enable_tcp && _l4proto+=("tcp")
    $enable_udp && _l4proto+=("udp")
    local l4proto=$(IFS=,; echo "${_l4proto[*]}")

    local _nfproto=()
    $enable_ipv4 && _nfproto+=("ipv4")
    $enable_ipv6 && _nfproto+=("ipv6")
    local nfproto=$(IFS=,; echo "${_nfproto[*]}")

    nft -f - <<EOF
table inet cgproxy {

    chain tproxy_ent {
        # core
        socket wildcard 0 mark set $fwmark_tproxy accept
        meta l4proto { tcp, udp } tproxy ip to 127.0.0.1:$port meta mark set $fwmark_tproxy
        meta l4proto { tcp, udp } tproxy ip6 to [::1]:$port meta mark set $fwmark_tproxy
    }

    chain tproxy_pre {
        type filter hook prerouting priority mangle - 5; policy accept;
        # filter
        fib daddr type local return
        fib daddr type != unicast return
        $($enable_gateway || echo "fib saddr type != local return")
        $($enable_dns && echo "udp dport 53 jump tproxy_ent")
        $([ -n "$l4proto" ] && echo "meta l4proto { $l4proto } jump tproxy_ent")
    }

    chain output {
        # hook
        type route hook output priority mangle - 5; policy accept;
        $([ -n "$nfproto" ] && echo "meta nfproto { $nfproto } jump tproxy_out" )
    }

    chain tproxy_mark {
        # filter
        fib daddr type != unicast return
        $($enable_dns && echo "udp dport 53 mark set $fwmark_tproxy")
        $([ -n "$l4proto" ] && echo "meta l4proto { $l4proto } mark set $fwmark_tproxy")
    }

    chain tproxy_out {
        # cgroup
        ct direction reply return
        $(
            for cg in ${cgroup_noproxy[@]}; do
                level=$(echo ${cg#/} | awk -F/ '{print NF}')
                echo "socket cgroupv2 level $level \"${cg#/}\" return"
            done
        )
        $(
            for cg in ${cgroup_proxy[@]}; do
                level=$(echo ${cg#/} | awk -F/ '{print NF}')
                echo "socket cgroupv2 level $level \"${cg#/}\" jump tproxy_mark"
            done
        )
    }
}
EOF

    ## mangle prerouting
    ip rule add fwmark $fwmark_tproxy table $table_tproxy
    ip route add local default dev lo table $table_tproxy
    ip -6 rule add fwmark $fwmark_tproxy table $table_tproxy
    ip -6 route add local default dev lo table $table_tproxy

    ## mangle output
    if [ $fwmark_reroute != $fwmark_tproxy ]; then
    ip rule add fwmark $fwmark_reroute table $table_reroute
    ip route add local default dev lo table $table_reroute
    ip -6 rule add fwmark $fwmark_reroute table $table_reroute
    ip -6 route add local default dev lo table $table_reroute
    fi

    ## forward #######################################################################
    if $enable_gateway; then
        echo "warning: gateway is not supported now"
    fi
}

load_base_ruleset(){
    ## ipv4 #########################################################################
    ## mangle prerouting
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
    { $enable_dns && ! $enable_udp; } && iptables -w 60 -t mangle -A TPROXY_PRE -p udp --dport 53 -j TPROXY_ENT
    $enable_udp && iptables -w 60 -t mangle -A TPROXY_PRE -p udp -j TPROXY_ENT
    $enable_tcp && iptables -w 60 -t mangle -A TPROXY_PRE -p tcp -j TPROXY_ENT
    # hook
    iptables -w 60 -t mangle -A PREROUTING -j TPROXY_PRE

    ## ipv6 #########################################################################
    ## mangle output
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
    { $enable_dns && ! $enable_udp; } && ip6tables -w 60 -t mangle -A TPROXY_PRE -p udp --dport 53 -j TPROXY_ENT
    $enable_udp && ip6tables -w 60 -t mangle -A TPROXY_PRE -p udp -j TPROXY_ENT
    $enable_tcp && ip6tables -w 60 -t mangle -A TPROXY_PRE -p tcp -j TPROXY_ENT
    # hook
    ip6tables -w 60 -t mangle -A PREROUTING -j TPROXY_PRE
}

load_local_ruleset(){
    ## ipv4 #########################################################################
    ## mangle prerouting
    # filter
    iptables -w 60 -t mangle -N TPROXY_MARK
    iptables -w 60 -t mangle -A TPROXY_MARK -m addrtype ! --dst-type UNICAST -j RETURN
    { $enable_dns && ! $enable_udp; } && iptables -w 60 -t mangle -A TPROXY_MARK -p udp --dport 53 -j MARK --set-mark $fwmark_reroute
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
    ## mangle output
    # filter
    ip6tables -w 60 -t mangle -N TPROXY_MARK
    ip6tables -w 60 -t mangle -A TPROXY_MARK -m addrtype ! --dst-type UNICAST -j RETURN
    { $enable_dns && ! $enable_udp; } && ip6tables -w 60 -t mangle -A TPROXY_MARK -p udp --dport 53 -j MARK --set-mark $fwmark_reroute
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
}

load_gateway_ruleset(){
    ## forward #######################################################################
    iptables  -t nat -A POSTROUTING -m owner ! --socket-exists -j MASQUERADE
    ip6tables -w 60 -t nat -A POSTROUTING -m owner ! --socket-exists -s fc00::/7 -j MASQUERADE # only masquerade ipv6 private address
}



unload_ruleset(){
    iptables -w 60 -t mangle -L TPROXY_ENT &> /dev/null || return
    echo "${self_name}: cleaning ${packetfilter_backend:-iptables} tproxy ruleset"

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
}

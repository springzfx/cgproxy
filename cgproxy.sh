#!/bin/bash

config="/etc/cgproxy.conf"
source $config

# test suid bit
if [ -u "$(which cgattach)" ]; then 
    cgattach $$ $proxy_cgroup && attached=1
else
    sudo cgattach $$ $proxy_cgroup && attached=1
fi

# test attach success or not
[[ -z "$attached" ]] && echo "config error" && exit 1

exec "$@"
#!/bin/bash

config="/etc/cgproxy.conf"
source $config

# test suid bit
if [ -u "$(which cgattach)" ]; then 
    cgattach $$ $proxy_cgroup
else
    sudo cgattach $$ $proxy_cgroup
fi
$@
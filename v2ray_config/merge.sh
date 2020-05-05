#!/bin/bash
jq -rs 'reduce .[] as $item ({}; . + $item  + {inbounds: (.inbounds + $item.inbounds)} + {outbounds: ($item.outbounds + .outbounds)})' *.json |sudo tee /etc/v2ray/config.json > /dev/null

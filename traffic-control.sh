#!/bin/bash

usage() {
    echo "Usage:"
    echo "  $0 add <bandwidth> <latency>"
    echo "  $0 clear"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

case "$1" in
    add)
        if [ $# -ne 3 ]; then
            usage
        fi
        BANDWIDTH=$2
        LATENCY=$3
        sudo tc qdisc add dev eth0 root netem rate "${BANDWIDTH}mbit" latency "${LATENCY}ms"
        ;;
    clear)
        if [ $# -ne 1 ]; then
            usage
        fi
        sudo tc qdisc del dev eth0 root
        ;;
    *)
        usage
        ;;
esac
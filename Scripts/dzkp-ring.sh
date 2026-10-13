#!/bin/bash

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

export PLAYERS=3

. $HERE/run-common.sh

has_lambda=false
has_q=false
for arg in "$@"; do
    case "$arg" in
        -lm|--lambda|-lm=*|--lambda=*) has_lambda=true ;;
        -q|--q-size|-q=*|--q-size=*) has_q=true ;;
    esac
done

$has_lambda || set -- "$@" -lm 2
$has_q || set -- "$@" -q 16

run_player dzkp-ring-party.x "$@" || exit 1

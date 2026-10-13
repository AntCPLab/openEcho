#!/usr/bin/env bash

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

rm Player-Data/3-echo-*/*
rm Player-Data/echo-bin-prep-data-{0,1,2}

export PLAYERS=3

. $HERE/run-common.sh

for dir in Player-Data/3-echo-{{R,B}-64,2-{40,128}}; do
    mkdir $dir 2> /dev/null
    rm $dir/*
    for t in $(seq 0 100); do
	touch $dir/{Protocol,Outputs}-{edaBits-,}P{0,1,2}-T$t
    done
done

touch Player-Data/echo-bin-prep-data-P{0,1,2}

run_player echo-prep-party.x $* || exit 1

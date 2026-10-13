#!/usr/bin/env bash

HERE=$(cd `dirname $0`; pwd)
SPDZROOT=$HERE/..

rm Player-Data/3-astra-*/*
for dir in Player-Data/3-astra-{{R,B}-64,2-{40,128}}; do
    mkdir $dir 2> /dev/null
    rm $dir/*
    for t in $(seq 0 100); do
	touch $dir/{Protocol,Outputs}-{edaBits-,}P{0,1,2}-T$t
    done
done

export PLAYERS=3

. $HERE/run-common.sh


run_player astra-prep-party.x $* || exit 1

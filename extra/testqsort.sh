#!/usr/bin/env bash

set -o xtrace
set -o errexit
set -o nounset
set -o pipefail

./mk.sh

C=100
for i in `seq 1 "$C"`
do
  N=$((RANDOM%100))
  for i in `seq 1 "$N"`
  do
    L=$((RANDOM%7))
    pwgen -s "$L" 1
  done | tee /tmp/testcase
  
  ./build/x 10 </tmp/testcase
done

echo "+OK (done)"

#!/bin/bash

# exit on error
set -e

# work in test directory
cd "$(dirname "$0")"

# import aliases / functions
source ../rfun

# each tool should be able to spit out a help text
tools="rdump rget rput rmon"
for tool in $tools ; do
    $tool -h > /dev/null
done

# create the example database
./demo_basics > /dev/null

# rdump
rdump | diff - demo_rdump.txt

# rget
rget -a 2 ROBOT_DATA | diff - demo_rget.txt

# rput
rput -a 2 TEST_FLOAT 7.0


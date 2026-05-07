#!/bin/bash


mkdir -p ../build
pushd ../build >> /dev/null
gcc ../source/iup_rbyedit.c -o debug -g -liup
popd >> /dev/null

#!/usr/bin/env bash

TMPDIR=$(pwd)

cd $TESTDIR
cp proj1 $TMPDIR
cd $TMPDIR
alias run="valgrind -q ./proj1"

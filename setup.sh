#!/usr/bin/env bash

TMPDIR=$(pwd)

cd $TESTDIR
cp AnotherTry $TMPDIR
alias run="valgrind -q ./AnotherTry"
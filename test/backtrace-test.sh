#!/bin/sh

OUT=$(test/backtrace-test-raise 2>&1)
echo "$OUT"
echo "$OUT" | grep 'in main backtrace-test-raise.cc:4'

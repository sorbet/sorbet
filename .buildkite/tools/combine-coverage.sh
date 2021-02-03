#!/bin/bash

mkdir -p "_tmp_"
OUT_FILE1=$(mktemp ./_tmp_/coverage.XXXXXX)
if [ -f "./_tmp_/profdata_combined.profdata" ]; then
  ./bazel-app/external/llvm_toolchain_10_0_0/bin/llvm-profdata merge -o="${OUT_FILE1}" ./_tmp_/profdata_combined.profdata "$@"
else
  ./bazel-app/external/llvm_toolchain_10_0_0/bin/llvm-profdata merge -o="${OUT_FILE1}" "$@"
fi

mv "${OUT_FILE1}" ./_tmp_/profdata_combined.profdata

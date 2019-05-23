#!/bin/bash

mkdir -p "_tmp_"
OUT_FILE1=$(mktemp ./_tmp_/coverage.XXXXXX)
if [ -f "./_tmp_/profdata_combined.profdata" ]; then
  ./bazel-sorbet/external/llvm_toolchain/bin/llvm-profdata merge -o="${OUT_FILE1}" ./_tmp_/profdata_combined.profdata "$@"
else
  ./bazel-sorbet/external/llvm_toolchain/bin/llvm-profdata merge -o="${OUT_FILE1}" "$@"
fi

mv "${OUT_FILE1}" ./_tmp_/profdata_combined.profdata

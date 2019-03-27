#!/bin/bash

mkdir -p "_tmp_"
CURRENT_FILES=$(ls ./_tmp_/profdata_combined.profdata)
OUT_FILE1=$(mktemp ./_tmp_/coverage.XXXXXX)
./bazel-sorbet/external/llvm_toolchain/bin/llvm-profdata merge -o=${OUT_FILE1} $CURRENT_FILES $@
mv ${OUT_FILE1} ./_tmp_/profdata_combined.profdata

#!/bin/bash
set -euo pipefail
EM_CONFIG="LLVM_ROOT='${PWD}/external/emscripten_clang_darwin/';"
EM_CONFIG+="EMSCRIPTEN_NATIVE_OPTIMIZER='${PWD}/external/external/emscripten_clang_darwin/optimizer';"
EM_CONFIG+="BINARYEN_ROOT='${PWD}/external/emscripten_clang_darwin/binaryen';"
EM_CONFIG+="NODE_JS='node';"
EM_CONFIG+="EMSCRIPTEN_ROOT='${PWD}/external/emscripten_toolchain';"
EM_CONFIG+="SPIDERMONKEY_ENGINE='';"
EM_CONFIG+="V8_ENGINE='';"
EM_CONFIG+="TEMP_DIR='${PWD}/tmp/emscripten_tmp';"
EM_CONFIG+="COMPILER_ENGINE=NODE_JS;"
EM_CONFIG+="JS_ENGINES=[NODE_JS];"
export EM_CONFIG

export EM_EXCLUSIVE_CACHE_ACCESS=1
export EMCC_SKIP_SANITY_CHECK=1
# export EMCC_DEBUG=2
export EMCC_WASM_BACKEND=0
mkdir -p "tmp/emscripten_tmp"

python external/emscripten_toolchain/emar.py "$@"
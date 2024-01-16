#!/bin/bash
set -euo pipefail

command -v realpath > /dev/null 2>&1 || { echo 'You need to install the "realpath" command system-wide.' ; exit 1; }

ABSOLUTE_PREFIX=$(realpath "${PWD}") # bazel replaces PWD with /proc/self/cwd which is unstable under "cd", meaning that referring to a path under relative name fails

EM_CONFIG="LLVM_ROOT='${ABSOLUTE_PREFIX}/external/emscripten_clang_linux/';"
EM_CONFIG+="EMSCRIPTEN_NATIVE_OPTIMIZER='${ABSOLUTE_PREFIX}/external/external/emscripten_clang_linux/optimizer';"
EM_CONFIG+="BINARYEN_ROOT='${ABSOLUTE_PREFIX}/external/emscripten_clang_linux/binaryen';"
EM_CONFIG+="NODE_JS='${ABSOLUTE_PREFIX}/external/nodejs_linux_amd64/bin/node';"
EM_CONFIG+="EMSCRIPTEN_ROOT='${ABSOLUTE_PREFIX}/external/emscripten_toolchain';"
EM_CONFIG+="SPIDERMONKEY_ENGINE='';"
EM_CONFIG+="V8_ENGINE='';"
EM_CONFIG+="TEMP_DIR='${ABSOLUTE_PREFIX}/tmp/emscripten_tmp';"
EM_CONFIG+="COMPILER_ENGINE=NODE_JS;"
EM_CONFIG+="JS_ENGINES=[NODE_JS];"
export EM_CONFIG

# Try to find Python on the system.
# Our version of emscripten uses Python 2, but upstream has already switched to
# Python 3. For now we prefer finding python2 if available, and fall back to
# trying Python 3 in case no Python 2 was found (which will chunder warnings
# but still work).
if command -v python &> /dev/null; then
  PYTHON="python"
elif command -v python2 &> /dev/null; then
  PYTHON="python2"
elif command -v python3 &> /dev/null; then
  PYTHON="python3"
else
  export PATH="${PATH}:/usr/local/bin/"
  PYTHON="python"
fi

export EM_EXCLUSIVE_CACHE_ACCESS=1
export EMCC_SKIP_SANITY_CHECK=1
# export EMCC_DEBUG=2
export EMCC_WASM_BACKEND=0
mkdir -p "tmp/emscripten_tmp"

"$PYTHON" external/emscripten_toolchain/emar.py "$@"

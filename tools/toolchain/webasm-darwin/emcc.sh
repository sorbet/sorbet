#!/bin/bash
set -eo pipefail

export PATH="${PATH}:/usr/local/bin/" # this is where we find node

command -v node >/dev/null 2>&1 || { echo 'will need node' ; exit 1; }
command -v realpath > /dev/null 2>&1 || { echo 'will need realpath' ; exit 1; }

EM_CONFIG="LLVM_ROOT='${PWD}/external/emscripten_clang_darwin/';"
EM_CONFIG+="EMSCRIPTEN_NATIVE_OPTIMIZER='${PWD}/external/external/emscripten_clang_darwin/optimizer';"
EM_CONFIG+="BINARYEN_ROOT='${PWD}/external/emscripten_clang_darwin/binaryen';"
EM_CONFIG+="NODE_JS='node';"
EM_CONFIG+="EMSCRIPTEN_ROOT='${PWD}/external/emscripten_toolchain';"
EM_CONFIG+="SPIDERMONKEY_ENGINE='';"
EM_CONFIG+="V8_ENGINE='';"
EM_CONFIG+="COMPILER_ENGINE=NODE_JS;"
EM_CONFIG+="JS_ENGINES=[NODE_JS];"


export EM_EXCLUSIVE_CACHE_ACCESS=1
export EMCC_SKIP_SANITY_CHECK=1
# export EMCC_DEBUG=2
export EMCC_WASM_BACKEND=0


export EM_EXCLUSIVE_CACHE_ACCESS=1
#export EMCC_SKIP_SANITY_CHECK=1
export EMCC_WASM_BACKEND=0

mkdir -p "tmp"
TMPDIR=$(realpath "tmp")
export TMPDIR
BC_RENAME_PREFIX=$(mktemp -dt "renaming_links-XXXX")
export BC_RENAME_PREFIX
EM_CACHE=$(mktemp -dt "emscripten_cache-XXXX")
export EM_CACHE
OUT_DIR=$(mktemp -dt "emscripten_out-XXXX")
export OUT_DIR
EMCC_TEMP_DIR=$(mktemp -dt "emscripten_tmp-XXXX")
export EMCC_TEMP_DIR
# shellcheck disable=SC2089
# ^^^^ complains that we have literal ' in string, that we _intended_ to have there
# I didn't find a way to structure it so that it stops complaining.
EM_CONFIG+="TEMP_DIR='${EMCC_TEMP_DIR}';"
# shellcheck disable=SC2090
# ^^^^ false positive due to false positive above
export EM_CONFIG
args=()
tar_name=
# bazel hardcodes `.o` files and `.a` files as outputs of tasks (WHY???).
# LLVM is happy to read&write bitcode to `.o` files
# but explicitly prohibits this for `.a` files. Fool it by creating symbolic links.
for i in "$@"; do
    if [[ "$i" =~ ^(.*)\.tar$ ]]; then
        tar_name="${BASH_REMATCH[1]}"
        folder_name=$(dirname "$i")
        mkdir -p "${OUT_DIR}/${folder_name}"
        out_name="${OUT_DIR}/${tar_name}.html"
        args=("${args[@]}" "$out_name")

    elif [[ "$i" =~ ^(.*)\.a$ ]]; then
        core_name="${BASH_REMATCH[1]}"
        folder_name=$(dirname "$i")
        mkdir -p "$BC_RENAME_PREFIX/$folder_name"
        link_name="$BC_RENAME_PREFIX/${core_name}.bc"
        cp "$i" "$link_name"
        args=("${args[@]}" "$link_name")
    elif [[ "$i" =~ ^-march=(.*)$ ]]; then
        # nothing
        continue
    else
        args=("${args[@]}" "$i")
    fi
done

# Run emscripten to compile and link
#echo "original_args" "$@"
#echo "transformed_args" "${args[@]}"
python external/emscripten_toolchain/emcc.py "${args[@]}"

if [ -n "${tar_name}" ]; then
    full_tar_path=$(realpath "${tar_name}.tar")
    folder_prefix=$(dirname "${OUT_DIR}/${tar_name}")
    file_name=$(basename "${OUT_DIR}/${tar_name}")
    cd "${folder_prefix}"
    tar cf "${full_tar_path}" ./"${file_name}"*
fi

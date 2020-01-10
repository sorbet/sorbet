package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
    name = "webasm-linux",
    tags = ["manual"],
    toolchains = {
        "webasm": ":webasm_toolchain_linux",
        "webasm-linux": ":webasm_toolchain_linux",
        "webasm|emscripten": ":webasm_toolchain_linux",
        "webasm|webasm": ":webasm_toolchain_linux",
    },
)

filegroup(name = "empty")

GENERATE_EM_CACHE_COMMAND = """
export EM_CACHE=$$(mktemp -d)
export PATH="$${PATH}:/usr/local/bin/"
EM_CONFIG="LLVM_ROOT='$${PWD}/external/emscripten_clang_linux/';"
EM_CONFIG+="EMSCRIPTEN_NATIVE_OPTIMIZER='$${PWD}/external/external/emscripten_clang_linux/optimizer';"
EM_CONFIG+="BINARYEN_ROOT='$${PWD}/external/emscripten_clang_linux/binaryen';"
EM_CONFIG+="NODE_JS='node';"
EM_CONFIG+="EMSCRIPTEN_ROOT='$${PWD}/external/emscripten_toolchain';"
EM_CONFIG+="SPIDERMONKEY_ENGINE='';"
EM_CONFIG+="V8_ENGINE='';"
EM_CONFIG+="COMPILER_ENGINE=NODE_JS;"
EM_CONFIG+="JS_ENGINES=[NODE_JS];"
export EM_EXCLUSIVE_CACHE_ACCESS=1
export EMCC_SKIP_SANITY_CHECK=1
export EMCC_WASM_BACKEND=0


export EM_EXCLUSIVE_CACHE_ACCESS=1
export EMCC_WASM_BACKEND=0

mkdir -p "tmp"
TMPDIR=$$(realpath "tmp")
export TMPDIR
BC_RENAME_PREFIX=$$(mktemp -dt "renaming_links-XXXX")
export BC_RENAME_PREFIX
OUT_DIR=$$(mktemp -dt "emscripten_out-XXXX")
export OUT_DIR
EMCC_TEMP_DIR=$$(mktemp -dt "emscripten_tmp-XXXX")
export EMCC_TEMP_DIR
EM_CONFIG+="TEMP_DIR='$${EMCC_TEMP_DIR}';"
export EM_CONFIG
python $${PWD}/external/emscripten_toolchain/embuilder.py build dlmalloc libc++_noexcept libc++abi libc libc-wasm libc-extras
tar -czf $(location em_cache.tar.gz) -C "$$EM_CACHE" .
"""

genrule(
    name = "generate_em_cache",
    srcs = [
        "@emscripten_clang_linux//:all",
        "@emscripten_toolchain//:all",
    ],
    outs = ["em_cache.tar.gz"],
    cmd = GENERATE_EM_CACHE_COMMAND,
    tags = ["manual"],
)

filegroup(
    name = "all-linux",
    srcs = [
        "em_cache_existing.tar.gz",
        "emcc.sh",
        "@emscripten_clang_linux//:all",
        "@emscripten_toolchain//:all",
    ],
    tags = ["manual"],
)

cc_toolchain(
    name = "webasm_toolchain_linux",
    all_files = ":all-linux",
    compiler_files = ":all-linux",
    dwp_files = ":empty",
    linker_files = ":all-linux",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 0,
    tags = ["manual"],
    toolchain_config = "wasm-linux",
    toolchain_identifier = "webasm-toolchain",
)

load(":cc_toolchain_config.bzl", "cc_toolchain_config")

cc_toolchain_config(
    name = "wasm-linux",
    cpu = "webasm",
)

package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
    name = "webasm-darwin",
    tags = ["manual"],
    toolchains = {
        "webasm": ":webasm_toolchain_darwin",
        "webasm-darwin": ":webasm_toolchain_darwin",
        "webasm|emscripten": ":webasm_toolchain_darwin",
        "webasm|webasm": ":webasm_toolchain_darwin",
    },
)

filegroup(name = "empty")

GENERATE_EM_CACHE_COMMAND = """
export EM_CACHE=$$(mktemp -d)
export PATH="$${PATH}:/usr/local/bin/"
EM_CONFIG="LLVM_ROOT='$${PWD}/external/emscripten_clang_darwin/';"
EM_CONFIG+="EMSCRIPTEN_NATIVE_OPTIMIZER='$${PWD}/external/external/emscripten_clang_darwin/optimizer';"
EM_CONFIG+="BINARYEN_ROOT='$${PWD}/external/emscripten_clang_darwin/binaryen';"
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
        "@emscripten_clang_darwin//:all",
        "@emscripten_toolchain//:all",
    ],
    outs = ["em_cache.tar.gz"],
    cmd = GENERATE_EM_CACHE_COMMAND,
    tags = ["manual"],
)

filegroup(
    name = "all-darwin",
    srcs = [
        "em_cache_existing.tar.gz",
        "emcc.sh",
        "@emscripten_clang_darwin//:all",
        "@emscripten_toolchain//:all",
    ],
)

cc_toolchain(
    name = "webasm_toolchain_darwin",
    all_files = ":all-darwin",
    compiler_files = ":all-darwin",
    cpu = "webasm-darwin",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":all-darwin",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
    tags = ["manual"],
    toolchain_identifier = "webasm-toolchain",
)

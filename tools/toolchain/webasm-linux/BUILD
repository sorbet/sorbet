package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
    name = "webasm-linux",
    tags = ["manual"],
    toolchains = {
        "webasm-linux": ":webasm_toolchain_linux",
        "webasm|emscripten": ":webasm_toolchain_linux",
    },
)

filegroup(name = "empty")

filegroup(
    name = "all-linux",
    srcs = [
        "emcc.sh",
        ":emscripten_cache_content_linux",
        "@emscripten_clang_linux//:all",
        "@emscripten_toolchain//:all",
    ],
)

filegroup(
    name = "emscripten_cache_content_linux",
    srcs = glob(["emscripten_cache_content_linux/**/*"]),
)

cc_toolchain(
    name = "webasm_toolchain_linux",
    all_files = ":all-linux",
    compiler_files = ":all-linux",
    cpu = "webasm-linux",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":all-linux",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 0,
    tags = ["manual"],
    toolchain_identifier = "webasm-toolchain",
)

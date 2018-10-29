package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
    name = "webasm-darwin",
    toolchains = {
        "webasm-darwin": ":webasm_toolchain_darwin",
        "webasm|emscripten": ":webasm_toolchain_darwin",
    },
)

filegroup(name = "empty")

filegroup(
    name = "all-darwin",
    srcs = [
        "emcc.sh",
        ":emscripten_cache_content_darwin",
        "@emscripten_clang_darwin//:all",
        "@emscripten_toolchain//:all",
    ],
)

filegroup(
    name = "emscripten_cache_content_darwin",
    srcs = glob(["emscripten_cache_content_darwin/**/*"]),
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
    toolchain_identifier = "webasm-toolchain",
)

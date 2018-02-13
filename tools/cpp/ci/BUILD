package(default_visibility = ["//visibility:public"])

filegroup(
    name = "clang_linux_all_files",
    srcs = [
        "//tools/cpp/ci/bin:wrappers",
        "@clang_5_0_0_linux//:bin",
        "@clang_5_0_0_linux//:includes",
        "@clang_5_0_0_linux//:libclang",
    ],
)

filegroup(
    name = "clang_linux_compiler_files",
    srcs = [
        "//tools/cpp/ci/bin:wrappers",
        "@clang_5_0_0_linux//:bin",
        "@clang_5_0_0_linux//:includes",
        "@clang_5_0_0_linux//:libclang",
    ],
)

filegroup(
    name = "empty",
    srcs = [],
)

cc_toolchain_suite(
    name = "toolchain",
    toolchains = {
        "local|compiler": ":cc-compiler-local",
    },
)

cc_toolchain(
    name = "cc-compiler-local",
    all_files = ":clang_linux_all_files",
    compiler_files = ":clang_linux_compiler_files",
    cpu = "local",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":clang_linux_compiler_files",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 1,
)

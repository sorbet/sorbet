sh_binary(
    name = "run",
    srcs = ["ruby"],
    data = [
        "compile",
        ":runtimeoverrides",
        "//main:sorbet",
        "@llvm_toolchain//:bin/ld",
    ],
    visibility = ["//test:__pkg__"],
)

filegroup(
    name = "runtimeoverrides",
    srcs = [
        "tools/patch_require.rb",
        "tools/preamble.rb",
    ],
    visibility = ["//test:__pkg__"],
)

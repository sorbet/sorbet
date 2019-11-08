sh_binary(
    name = "run",
    srcs = ["ruby"],
    data = [
        "compile",
        "tools/patch_require.rb",
        "tools/preamble.rb",
        "//main:sorbet",
        "@llvm_toolchain//:bin/ld",
    ],
    visibility = ["//test:__pkg__"],
)

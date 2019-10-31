sh_binary(
    name = "run",
    srcs = ["ruby"],
    data = [
        "compile",
        "patch_require.rb",
        "preamble.rb",
        "//main:sorbet",
        "@llvm_toolchain//:bin/ld",
    ],
    visibility = ["//test:__pkg__"],
)

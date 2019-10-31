sh_binary(
    name = "bin",
    srcs = ["ruby"],
    data = [
        "compile",
        "preamble.rb",
        "//main:sorbet",
        "@llvm_toolchain//:bin/ld",
    ],
    visibility = ["//test:__pkg__"],
)

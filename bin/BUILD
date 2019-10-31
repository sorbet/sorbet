sh_binary(
    name = "bin",
    srcs = ["ruby"],
    data = [
        "compile_and_run_one",
        "preamble.rb",
        "//main:sorbet",
        "@llvm_toolchain//:bin/ld",
    ],
    visibility = ["//visibility:public"],
)

test_suite(
    name = "test",
    tests = [
        "test_corpus",
        "//test/llvm:kaleidoscope_test",
    ],
)

load(":compiler_tests.bzl", "compiler_tests")

compiler_tests(
    glob([
        "testdata/**/*.rb",
    ]),
)

sh_binary(
    name = "test_corpus_runner",
    srcs = ["test_corpus_runner.sh"],
    data = [
        "preamble.rb",
        "//main:sorbet_llvm",
        "@llvm_toolchain//:bin/ld",
        "@llvm_toolchain//:bin/opt",
    ],
)

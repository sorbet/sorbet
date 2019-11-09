test_suite(
    name = "test",
    tests = [
        "test_corpus",
        "//test/llvm:kaleidoscope_test",
    ],
)

load(":compiler_tests.bzl", "compiler_tests")

compiler_tests(
    "test_corpus",
    glob([
        "testdata/llvm/**/*.rb",
    ]),
)

compiler_tests(
    "ruby_benchmark",
    glob([
        "testdata/ruby_benchmark/**/*.rb",
    ]),
)

sh_binary(
    name = "test_corpus_runner",
    srcs = ["test_corpus_runner.sh"],
    data = [
        "//run",
        "@ruby_2_6_3//:ruby",
    ],
)

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
        "//bin",
        "@ruby_2_6_3//:ruby",
    ],
)

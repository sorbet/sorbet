test_suite(
    name = "test",
    tests = ["//test/llvm:kaleidoscope_test", "test_corpus"],
)

load("@com_stripe_ruby_typer//test:pipeline_test.bzl", "pipeline_tests")

pipeline_tests(
    "test_corpus",
    glob([
        "testdata/**/*.rb",
    ]),
    "",
)

sh_binary(
    name = "test_corpus_runner",
    srcs = ["test_corpus_runner.sh"],
    data = ["//main:sorbet_llvm"],
)

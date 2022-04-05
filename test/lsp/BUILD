load(":lsp_test.bzl", "lsp_test", "protocol_tests", "update_test")

sh_binary(
    name = "lsp_test_runner",
    srcs = ["lsp_test_runner.sh"],
    data = ["//main:sorbet"],
)

protocol_tests([
    "cache_protocol_test_corpus.cc",
    "multithreaded_protocol_test_corpus.cc",
    "protocol_test_corpus.cc",
    "watchman_test_corpus.cc",
])

test_suite(
    name = "lsp",
    tests = [lsp_test(script) for script in glob(["*/*.rec"])],
)

update_test()

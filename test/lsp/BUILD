load(":lsp_test.bzl", "lsp_test", "update_test")

sh_binary(
    name = "lsp_test_runner",
    srcs = ["lsp_test_runner.sh"],
    data = ["//main:sorbet"],
)

test_suite(
    name = "lsp",
    tests = [lsp_test(script) for script in glob(["*/*.rec"])],
)

update_test()

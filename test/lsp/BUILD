load(":lsp_test.bzl", "lsp_test")

sh_binary(
    name = "lsp_test_runner",
    srcs = ["lsp_test_runner.sh"],
    data = ["//main:sorbet"],
)

[
    lsp_test(script)
    for script in glob(["*/*.rec"])
]

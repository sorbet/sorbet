
sh_test(
    name = "smoke_test",
    data = [
        "//bundler:bundle",
        "vendor/cache/cantor-1.2.1.gem",
        "Gemfile",
        "Gemfile.lock",
    ],
    deps = [
        "@bazel_tools//tools/bash/runfiles",
    ],
    srcs = [
        "smoke_test.sh",
    ]
)

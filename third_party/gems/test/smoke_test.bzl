def smoke_tests(versions):
    for version in versions:
        native.sh_test(
            name = "smoke_test-{}".format(version),
            data = [
                "//bundler:bundle",
                "vendor/cache/cantor-1.2.1.gem",
                "Gemfile",
                "Gemfile.lock",
                "@{}//:ruby".format(version),
            ],
            deps = [
                "@bazel_tools//tools/bash/runfiles",
            ],
            srcs = ["smoke_test.sh"],
            args = [version],
        )


def smoke_tests(versions):
    for version in versions:
        native.sh_test(
            name = "smoke_test-{}".format(version),
            data = [
                "//bundler:bundle",
                "vendor/cache/cantor-1.2.1.gem",
                "Gemfile",
                "Gemfile.lock",
            ],
            deps = [
                "@bazel_tools//tools/bash/runfiles",
                "@{}//:ruby".format(version),
            ],
            srcs = ["smoke_test.sh"],
            args = [version],
        )

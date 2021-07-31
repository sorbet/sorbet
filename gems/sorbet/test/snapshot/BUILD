load(":snapshot.bzl", "snapshot_tests")

sh_library(
    name = "logging",
    srcs = ["logging.sh"],
)

sh_library(
    name = "hermetic_tar",
    srcs = ["hermetic_tar.sh"],
)

sh_library(
    name = "validate_utils",
    srcs = ["validate_utils.sh"],
    deps = [":logging"],
)

sh_binary(
    name = "run_one",
    srcs = ["run_one.sh"],
    data = [
        "sorbet-typed.rev",
        "//main:sorbet",
        "@gems//gems",
        "@sorbet_ruby_2_6//:bundle",
        "@sorbet_ruby_2_6//:ruby",
        "@sorbet_ruby_2_7//:bundle",
        "@sorbet_ruby_2_7//:ruby",
    ],
    # NOTE: ensure that this rule isn't caught in //...
    tags = ["manual"],
    deps = [
        ":hermetic_tar",
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

snapshot_tests(
    ruby = "sorbet_ruby_2_6",
    tests = glob(
        [
            "partial/*",
            "total/*",
        ],

        # We only want to match directories with this glob
        exclude_directories = 0,
    ),
)

snapshot_tests(
    ruby = "sorbet_ruby_2_7",
    tests = glob(
        [
            "partial/*",
            "total/*",
        ],

        # We only want to match directories with this glob
        exclude_directories = 0,
    ),
)

test_suite(
    name = "snapshot",
    tests = [
        ":snapshot-sorbet_ruby_2_6",
        ":snapshot-sorbet_ruby_2_7",
    ],
)

test_suite(
    name = "update",
    tests = [
        ":update-sorbet_ruby_2_6",
        ":update-sorbet_ruby_2_7",
    ],
)

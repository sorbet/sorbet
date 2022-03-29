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
        "@sorbet_ruby_3_0//:bundle",
        "@sorbet_ruby_3_0//:ruby",
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

snapshot_tests(
    ruby = "sorbet_ruby_3_0",
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

    # NOTE: this is manual to avoid being caught with `//...`
    tags = ["manual"],
    tests = [
        ":snapshot-sorbet_ruby_2_6",
        ":snapshot-sorbet_ruby_2_7",
        ":snapshot-sorbet_ruby_3_0",
    ],
)

test_suite(
    name = "update",

    # NOTE: this is manual to avoid being caught with `//...`
    tags = ["manual"],
    tests = [
        ":update-sorbet_ruby_2_6",
        ":update-sorbet_ruby_2_7",
        ":update-sorbet_ruby_3_0",
    ],
)

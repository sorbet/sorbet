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
        "@ruby_2_5//:bundle",
        "@ruby_2_5//:ruby",
        "@ruby_2_6//:bundle",
        "@ruby_2_6//:ruby",
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
    ruby = "ruby_2_5",
    tests = glob(
        [
            "partial/*",
            "total/*",
        ],
        exclude = [
            # Requires native extensions.
            # Can't be rewritten, because it's essentially an integration test
            # that a certain private Rails API gives us back absolute vs
            # relative paths.
            "partial/rails-double-require",

            # Requires ruby 2.6.
            "partial/rails6",
        ],

        # We only want to match directories with this glob
        exclude_directories = 0,
    ),
)

snapshot_tests(
    ruby = "ruby_2_6",
    tests = glob(
        [
            "partial/*",
            "total/*",
        ],
        exclude = [
            # Requires native extensions.
            # Can't be rewritten, because it's essentially an integration test
            # that a certain private Rails API gives us back absolute vs
            # relative paths.
            "partial/rails-double-require",

            # Requires nokogiri, which builds a native extension
            "partial/rails6",
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
        ":snapshot-ruby_2_5",
        ":snapshot-ruby_2_6",
    ],
)

test_suite(
    name = "update",

    # NOTE: this is manual to avoid being caught with `//...`
    tags = ["manual"],
    tests = [
        ":update-ruby_2_5",
        ":update-ruby_2_6",
    ],
)

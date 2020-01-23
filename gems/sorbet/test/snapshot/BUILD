load(":snapshot.bzl", "snapshot_tests")

sh_library(
    name = "logging",
    srcs = ["logging.sh"],
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
        "@gems//bundler",
        "@gems//bundler:bundle",
        "@gems//gems",
        "@ruby_2_4//:ruby",
        "@ruby_2_6//:ruby",
    ],
    # NOTE: ensure that this rule isn't caught in //...
    tags = ["manual"],
    deps = [
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

snapshot_tests(
    ruby = "ruby_2_4",
    tests = glob(
        [
            "partial/*",
            "total/*",
        ],
        exclude = [
            # These tests can't currently run under bazel,
            # and can't be written another way.

            # Requires openssl, which doesn't work in bazel.
            # Can't be rewritten because it's testing gem_loader.
            "partial/webmock",

            # Requires native extensions. Tests gem_loader for codecov.
            "partial/codecov",

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
            # These tests can't currently run under bazel,
            # and can't be written another way.

            # Requires openssl, which doesn't work in bazel.
            # Can't be rewritten because it's testing gem_loader.
            "partial/webmock",

            # Requires native extensions. Tests gem_loader for codecov.
            "partial/codecov",

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
        ":snapshot-ruby_2_4",
        ":snapshot-ruby_2_6",
    ],
)

test_suite(
    name = "update",

    # NOTE: this is manual to avoid being caught with `//...`
    tags = ["manual"],
    tests = [
        ":update-ruby_2_4",
        ":update-ruby_2_6",
    ],
)

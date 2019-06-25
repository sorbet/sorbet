load(":snapshot.bzl", "snapshot_tests")

sh_library(
    name = "logging",
    srcs = ["logging.sh"],
)

sh_binary(
    name = "run_one_bazel",
    srcs = ["run_one_bazel.sh"],
    data = [
        "//main:sorbet",
        "@gems//bundler",
        "@gems//bundler:bundle",
        "@gems//gems",
        "@ruby_2_4_3//:ruby",
    ],
    deps = [
        ":logging",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

snapshot_tests(
    name = "snapshot",
    tests = glob(
        [
            "partial/*",
            "total/*",
        ],
        exclude = [
            # openssl is missing
            "partial/webmock",
            "partial/partially-sigged",

            # native extensions don't build
            "partial/codecov",
            "partial/rails-double-require",
        ],

        # We only want to match directories with this glob
        exclude_directories = 0,
    ),
    update_name = "update",
)

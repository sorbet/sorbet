filegroup(
    name = "runtime-files",
    srcs = glob([
        "lib/**/*.rb",
    ]) + [
        "Rakefile",
        "Gemfile",
        "sorbet-runtime.gemspec",
    ],
)

filegroup(
    name = "runtime-files-test",
    srcs = glob([
        "test/types/*.rb",
    ]),
)

filegroup(
    name = "runtime-stubs",
    srcs = ["test/test_helper.rb"],
)

sh_test(
    name = "runtime-typechecks",
    srcs = ["test/typecheck-runtime.sh"],
    args = [
        "$(locations runtime-files)",
        "$(location runtime-stubs)",
    ],
    data = [
        ":runtime-files",
        ":runtime-stubs",
        "//main:sorbet",
    ],
)

filegroup(
    name = "rbi",
    srcs = glob(
        [
            "**/*.rb",
            "**/*.rbi",
        ],
    ),
    visibility = ["//visibility:public"],
)

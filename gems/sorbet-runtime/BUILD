filegroup(
    name = "sorbet-runtime",
    srcs = glob([
        "lib/**/*.rb",
    ]) + [
        "Rakefile",
        "Gemfile",
        "sorbet-runtime.gemspec",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "runtime-files-test",
    srcs = glob([
        "test/types/*.rb",
    ]),
)

filegroup(
    name = "sorbet-runtime-stubs",
    srcs = ["test/test_helper.rb"] + glob(["test/*.rbi"]),
)

sh_test(
    name = "runtime-typechecks",
    srcs = ["test/typecheck-runtime.sh"],
    args = [
        "$(locations sorbet-runtime)",
        "$(locations sorbet-runtime-stubs)",
    ],
    data = [
        ":sorbet-runtime",
        ":sorbet-runtime-stubs",
        "//main:sorbet",
    ],
)

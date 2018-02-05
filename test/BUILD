cc_binary(
    name = "test_corpus_sharded",
    testonly = 1,
    srcs = [
        "test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "//ast/treemap",
        "//cfg",
        "//cfg/builder",
        "//core",
        "//core/serialize",
        "//infer",
        "//namer",
        "//payload/binary",
        "//resolver",
        "@cxxopts",
        "@gtest",
    ],
)

filegroup(
    name = "gerald",
    srcs = [
        "testdata/first/gerald.rb",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "hello-test",
    size = "small",
    srcs = ["hello-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//common",
        "//core",
        "//parser",
        "@cxxopts",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "error-check-test",
    size = "small",
    srcs = ["error-check-test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//common",
        "//core",
        "//parser",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

# Passes with --config=dbg but I'm not smart enough to figure out how to make it
# only run when in that config
# sh_test(
#     name = "backtrace-test",
#     size = "small",
#     srcs = ["backtrace-test.sh"],
#     data = ["backtrace-test-raise"],
# )

cc_binary(
    name = "backtrace-test-raise",
    srcs = ["backtrace-test-raise.cc"],
    copts = ["--std=c++14"],
    visibility = ["//tools:__pkg__"],
)

cc_binary(
    name = "backtrace-test-error",
    srcs = ["backtrace-test-error.cc"],
    copts = ["--std=c++14"],
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

sh_test(
    name = "end-to-end-test",
    size = "small",
    srcs = ["end-to-end-test.sh"],
    data = [
        "end-to-end-test.out",
        "end-to-end-test-input.rb",
        "//main:ruby-typer",
    ],
)

sh_test(
    name = "dash-e-test",
    size = "small",
    srcs = ["dash-e-test.sh"],
    data = [
        "dash-e-test.out",
        "//main:ruby-typer",
    ],
)

SHARDS = 8

[
    sh_test(
        name = "test_corpus{}".format(shard),
        size = "small",
        srcs = ["test_corpus_forwarder.sh"],
        args = [
            "--shards_total={}".format(SHARDS),
            "--my_id={}".format(shard),
        ],
        data = glob([
            "testdata/**/*.rb",
            "testdata/**/*.exp",
            "testdata/**/*.exp.svg",
        ]) + ["test_corpus_sharded"],
    )
    for shard in range(SHARDS)
]

test_suite(
    name = "test_corpus",
    tests = ["test_corpus{}".format(shard) for shard in range(SHARDS)],
)

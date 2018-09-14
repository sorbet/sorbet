config_setting(
    name = "jemalloc",
    values = {
        "define": "jemalloc=true",
        "compilation_mode": "opt",
    },
)

cc_binary(
    name = "sorbet",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkopts = select({
        "//tools/config:more-static": ["-static"],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    malloc = select({
        ":jemalloc": "@jemalloc//:jemalloc",
        "//conditions:default": "@bazel_tools//tools/cpp:malloc",
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary:some",
        "//payload/text:empty",
        "@com_google_absl//absl/debugging:failure_signal_handler",
    ],
)

cc_binary(
    name = "sorbet-orig",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary:empty",
        "//payload/text:some",
        "@com_google_absl//absl/debugging:failure_signal_handler",
    ],
)

cc_library(
    name = "realmain",
    srcs = [
        "realmain.cc",
        "realmain.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common/kvstore",
        "//common/statsd",
        "//core",
        "//core/proto",
        "//core/serialize",
        "//main/autogen",
        "//main/errorqueue",
        "//main/lsp",
        "//main/options",
        "//main/pipeline",
        "//payload/binary",
        "//payload/text",
    ],
)

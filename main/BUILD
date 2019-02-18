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
    features = select({
        "//tools/config:linkshared": ["fully_static_link"],
        "//conditions:default": [],
    }),
    # When changing this, you should also update the REQUIRED_STACK_SIZE variable in common/os/os.cc.
    linkopts = select({
        "//tools/config:darwindbg": [
            "-Wl,-stack_size",
            "-Wl,0x4000000",
        ],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
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
    ],
)

cc_binary(
    name = "sorbet-orig",
    srcs = [
        "main.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "//payload/binary:empty",
        "//payload/text:some",
    ],
)

cc_library(
    name = "realmain",
    srcs = [
        "realmain.cc",
        "realmain.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common/kvstore",
        "//common/statsd",
        "//core",
        "//core/proto",
        "//core/serialize",
        "//main/autogen",
        "//main/lsp",
        "//main/options",
        "//main/pipeline",
        "//payload/binary",
        "//payload/text",
        "@ruby",
    ],
)

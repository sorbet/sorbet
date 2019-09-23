config_setting(
    name = "jemalloc",
    values = {
        "define": "jemalloc=true",
        "compilation_mode": "opt",
    },
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "sorbet",
    srcs = [
        "main.cc",
    ],
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
        "//main/pipeline/semantic_extension:none",
        "//payload",
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
        "realmain-orig",
        "//main/pipeline/semantic_extension:none",
        "//payload:text",
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
        "//common/statsd",
        "//core",
        "//core/proto",
        "//main/autogen",
        "//main/lsp",
        "//main/options",
        "//main/pipeline",
        "//payload:interface",
    ],
)

cc_library(
    name = "realmain-orig",
    srcs = [
        "realmain.cc",
        "realmain.h",
    ],
    defines = ["SORBET_REALMAIN_MIN"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
        "//main/options",
        "//main/pipeline:pipeline-orig",
        "//payload:interface",
    ],
)

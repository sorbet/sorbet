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
        "realmain",
        "//payload:text",
    ],
)

cc_binary(
    name = "sorbet-orig-test-bootstrap",
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
        "//payload:test_bootstrap_text",
    ],
)

cc_binary(
    name = "sorbet-test-bootstrap",
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
        "//payload:test_bootstrap",
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

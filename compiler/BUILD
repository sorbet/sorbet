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
        "//main:jemalloc": "@jemalloc//:jemalloc",
        "//conditions:default": "@bazel_tools//tools/cpp:malloc",
    }),
    tags = ["compiler"],
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/ObjectFileEmitter",
        "//main:realmain",
        "//payload",
        "//plugin_injector",
    ],
)

test_suite(
    name = "compiler",
    tests = ["//test:compiler_oracle"],
)

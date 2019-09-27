cc_binary(
    name = "sorbet_llvm",
    srcs = [
        "main.cc",
        "llvm_main.h",
        "llvm_main.cc",
    ],
    # When changing this, you should also update the REQUIRED_STACK_SIZE variable in common/os/os.cc.
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwindbg": [
            "-Wl,-stack_size",
            "-Wl,0x4000000",
        ],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    malloc = select({
        "@com_stripe_ruby_typer//main:jemalloc": "@jemalloc//:jemalloc",
        "//conditions:default": "@bazel_tools//tools/cpp:malloc",
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//plugin_injector",
        "@com_stripe_ruby_typer//main:realmain",
        "@com_stripe_ruby_typer//payload",
    ],
)

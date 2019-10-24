cc_binary(
    name = "sorbet",
    srcs = [
        "main.cc",
    ],
    # When changing this, you should also update the REQUIRED_STACK_SIZE variable in common/os/os.cc.
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwindbg": [
            "-Wl,-stack_size",
            "-Wl,0x4000000",
        ],
        "//conditions:default": [],
    }),
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/ObjectFileEmitter",
        "//plugin_injector",
        "@com_stripe_ruby_typer//main:realmain",
        "@com_stripe_ruby_typer//payload",
    ],
)

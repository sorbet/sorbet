cc_library(
    name = "Passes",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    hdrs = ["Passes.h"],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Core",
        "//compiler/IREmitter/Payload",
        "@spdlog",
    ],
)

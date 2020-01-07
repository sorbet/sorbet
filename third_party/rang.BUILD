cc_library(
    name = "rang",
    srcs = [],
    hdrs = glob([
        "include/rang.hpp",
    ]),
    includes = [
        "include/",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

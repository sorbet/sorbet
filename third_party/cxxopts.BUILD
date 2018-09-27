cc_library(
    name = "cxxopts",
    srcs = [],
    hdrs = [
        "include/cxxopts.hpp",
    ],
    copts = [
        "-Iexternal/cxxopts/",
    ],
    includes = [
        "include/",
    ],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)

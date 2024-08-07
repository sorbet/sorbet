cc_library(
    name = "prism",
    srcs = [
        "Parser.cc",
    ],
    hdrs = [
        "Parser.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
    ],
)

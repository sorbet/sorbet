cc_library(
    name = "rbs",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common",
        "//core",
        "//parser",
        "//rewriter/util",
        "@rbs_parser",
    ],
)

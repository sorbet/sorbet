cc_library(
    name = "rbs",
    srcs = glob([
        "*.cc",
        "*.h",
        "prism/*.cc",
        "prism/*.h",
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
        "//main/options",
        "//parser",
        "//parser/prism",
        "//rewriter/util",
        "@rbs_parser",
    ],
)

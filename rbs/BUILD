cc_library(
    name = "rbs",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-Iexternal/rbs_parser/include",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common",
        "//core",
        "@rbs_parser",
    ],
)

cc_library(
    name = "helpers",
    testonly = 1,
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//main/lsp",
        "@doctest//doctest",
        "@dtl",
    ],
)

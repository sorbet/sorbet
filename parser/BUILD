cc_library(
    name = "parser",
    srcs = [
        "Builder.h",
    ] + glob(["*.cc"]),
    hdrs = [
        "Node.h",
        "Result.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "@parser//:parser",
    ],
)

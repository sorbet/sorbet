cc_library(
    name = "class_flatten",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = [
        "class_flatten.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//ast/treemap",
        "//core",
    ],
)

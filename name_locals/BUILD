cc_library(
    name = "name_locals",
    srcs = glob(
        ["*.cc"],
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = [
        "name_locals.h",
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

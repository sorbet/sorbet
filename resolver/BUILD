cc_library(
    name = "resolver",
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
        "resolver.h",
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

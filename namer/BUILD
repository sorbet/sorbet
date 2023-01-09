cc_library(
    name = "namer",
    srcs = glob(
        ["*.cc"],
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = [
        "namer.h",
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
        "//class_flatten",
        "//core",
    ],
)

cc_test(
    name = "namer_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "namer",
        "//ast/desugar",
        "//local_vars",
        "//parser",
        "//rewriter",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

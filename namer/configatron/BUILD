cc_library(
    name = "configatron",
    srcs = glob(
        ["*.cc"],
        exclude = [
            "*_test.cc",
            "flycheck_*",
        ],
    ),
    hdrs = [
        "configatron.h",
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
        "@yaml_cpp",
    ],
)

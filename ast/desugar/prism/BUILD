cc_library(
    name = "prism",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/desugar",
        "//ast/verifier",
        "//core",
        "//parser",
        "//parser/prism",
        "@prism",
    ],
)

cc_test(
    name = "concrete_syntax_test",
    size = "small",
    srcs = ["test/concrete_syntax_test.cc"],
    deps = [
        ":prism",
        "//ast",
        "//ast/desugar",
        "//core",
        "//parser",
        "//parser/prism",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

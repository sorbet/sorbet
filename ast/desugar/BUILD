cc_library(
    name = "desugar",
    srcs = glob(
        ["*.cc"],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = glob(["*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/verifier",
        "//core",
        "//parser",
    ],
)

cc_test(
    name = "desugar_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    data = glob([
        "fixtures/*.rb",
        "fixtures/*.rb.exp",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//ast/desugar",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

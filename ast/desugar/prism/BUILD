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
        "//parser/prism",
        "@prism",
    ],
)

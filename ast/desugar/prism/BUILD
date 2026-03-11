cc_library(
    name = "prism",
    srcs = ["Translator.cc"],
    hdrs = ["Translator.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//core",
        "//parser/prism",
        "@prism",
    ],
)

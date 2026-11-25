cc_library(
    name = "prism",
    srcs = [
        "Factory.cc",
        "Parser.cc",
    ],
    hdrs = [
        "Factory.h",
        "Helpers.h",
        "Parser.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//parser",  # Legacy parser, needed for translating to its nodes defined in `parser/Node_gen.h`
        "@prism",
    ],
)

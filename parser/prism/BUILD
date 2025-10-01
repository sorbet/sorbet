cc_library(
    name = "prism",
    srcs = [
        "Helpers.cc",
        "Parser.cc",
        "Translator.cc",
    ],
    hdrs = [
        "Helpers.h",
        "Parser.h",
        "Translator.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",  # Needed for `DuplicateHashKeyCheck.h`
        "//core",
        "//parser",  # Legacy parser, needed for translating to its nodes defined in `parser/Node_gen.h`
        "@prism",
    ],
)

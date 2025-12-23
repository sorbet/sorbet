cc_library(
    name = "autogen",
    srcs = [
        "autogen.cc",
        "crc_builder.cc",
        "subclasses.cc",
    ],
    hdrs = [
        "autogen.h",
        "crc_builder.h",
        "subclasses.h",
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
        "//main/autogen/data",
        "//main/options",
        "@crcpp",
    ],
)

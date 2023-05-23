cc_library(
    name = "autogen",
    srcs = [
        "autogen.cc",
        "cache.cc",
        "constant_hash.cc",
        "crc_builder.cc",
        "subclasses.cc",
    ],
    hdrs = [
        "autogen.h",
        "cache.h",
        "constant_hash.h",
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
        "@com_github_d_bahr_crcpp",
    ],
)

cc_test(
    name = "constant_hash_test",
    size = "small",
    srcs = ["test/constant_hash_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":autogen",
        "//ast/desugar",
        "//core",
        "//parser",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

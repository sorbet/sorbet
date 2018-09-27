cc_library(
    name = "autogen",
    srcs = [
        "autogen.cc",
    ],
    hdrs = [
        "autogen.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//core",
        "@com_github_d_bahr_crcpp",
        "@com_github_msgpack_msgpack",
    ],
)

cc_library(
    name = "autogen",
    srcs = [
        "autogen.cc",
        "autoloader.cc",
        "subclasses.cc",
    ],
    hdrs = [
        "autogen.h",
        "autoloader.h",
        "subclasses.h",
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
        "//main/autogen/data",
        "//main/options",
        "@com_github_d_bahr_crcpp",
    ],
)

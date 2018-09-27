cc_library(
    name = "options",
    srcs = [
        "FileFlatMapper.cc",
        "FileFlatMapper.h",
        "options.cc",
    ],
    hdrs = ["options.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//version",
        "@cxxopts",
        "@yaml_cpp",
    ],
)

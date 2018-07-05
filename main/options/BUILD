cc_library(
    name = "options",
    srcs = [
        "FileFlatMapper.cc",
        "FileFlatMapper.h",
        "options.cc",
    ],
    hdrs = ["options.h"],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//version",
        "@cxxopts",
        "@yaml_cpp",
    ],
)

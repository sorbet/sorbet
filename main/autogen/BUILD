cc_library(
    name = "autogen",
    srcs = [
        "autogen.cc",
    ],
    hdrs = [
        "autogen.h",
    ],
    copts = ["-std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//ast/treemap",
        "//core",
        "@com_github_d_bahr_crcpp",
        "@com_github_msgpack_msgpack",
    ],
)

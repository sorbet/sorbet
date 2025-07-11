load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

cmake(
    name = "mimalloc_lib",
    cache_entries = {
        "MI_INSTALL_TOPLEVEL": "ON",
    },
    lib_source = ":srcs",
    out_static_libs = ["libmimalloc.a"],
)

cc_library(
    name = "mimalloc",
    hdrs = ["include/mimalloc.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":mimalloc_lib",
    ],
)

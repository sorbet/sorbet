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
        # mimalloc v3 defaults allow_thp=1 on Linux and madvise()s MADV_HUGEPAGE
        # over its whole 1GiB arena; on THP=[madvise] kernels every touched 4KiB
        # slot then faults a 2MiB resident huge page, adding ~45MB of phantom
        # RSS to small/medium typechecks (#10889). Runtime opt-in stays
        # available via MIMALLOC_ALLOW_THP=1.
        "MI_NO_THP": "ON",
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

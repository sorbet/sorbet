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
        # Transparent hugepage support can be useful, but it drastically increases
        # RSS usage for Sorbet runs on small codebases, and even on large
        # codebases, it's not obvious that it's a win.  Folks who want to
        # experiment with transparent hugepages can set `MIMALLOC_ALLOW_THP=1`
        # when running Sorbet.
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

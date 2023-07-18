load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

# The MI_INSTALL_TOPLEVEL CMake flag is depracated
# https://github.com/microsoft/mimalloc/blob/4e50d6714d471b72b2285e25a3df6c92db944593/CMakeLists.txt#L33
# this could break at any point. The current default
# behavior is to append the mimalloc version. Eg:
# out_static_libs = ["mimalloc-2.1/libmimalloc.a"]
# If this flag breaks in the future, you may want to
# either hard-code the version or be more clever.
# find the static library in bazel-out to debug
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

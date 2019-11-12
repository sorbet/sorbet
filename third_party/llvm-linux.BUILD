cc_library(
    name = "org_llvm",
    # Don't include lib/ recursively, because we don't want all the clang code in there.
    srcs = glob([
        "lib/libclang-cpp.s",
    ]),
    visibility = ["//visibility:public"],
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.inc",
        "include/**/*.def",
        "include/**/*.gen",
    ]),
    includes = ["include/"],
    linkopts = [
        "-lpthread",
        "-lcurses",
        "-ldl",
        "-lz",
    ],
)

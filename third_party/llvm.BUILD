cc_library(
    name = "org_llvm",
    # Don't include lib/ recursively, because we don't want all the clang code in there.
    srcs = glob([
        "lib/libLLVM*.a",
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
        "-ldl",
        "-lz",
    ],
)

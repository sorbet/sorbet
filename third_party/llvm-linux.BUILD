cc_library(
    name = "org_llvm_linux",
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
        "-lcurses",
        "-ldl",
        "-lz",
        "-lstdc++",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
)

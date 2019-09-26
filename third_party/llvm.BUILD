cc_library(
    name = "org_llvm_darwin",
    # Don't include lib/ recursively, because we don't want all the clang code in there.
    srcs = [
        "lib/libLLVMCore.a",
        "lib/libLLVMBinaryFormat.a",
        "lib/libLLVMSupport.a",
        "lib/libLLVMDemangle.a",
    ],
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
    ],
)

cc_binary(
    name = "kaleidoscope",
    srcs = ["kaleidoscope.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "@org_llvm_darwin",
    ],
)

sh_test(
    name = "kaleidoscope_test",
    size = "small",
    srcs = ["kaleidoscope_test.sh"],
    data = [
        "kaleidoscope_test.out",
        ":kaleidoscope",
    ],
    visibility = ["//visibility:public"],
)

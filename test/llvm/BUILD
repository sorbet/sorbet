cc_binary(
    name = "kaleidoscope",
    srcs = ["kaleidoscope.cpp"],
    deps = [
        "@org_llvm_darwin",
    ],
)

sh_test(
    name = "kaleidoscope_test",
    srcs = ["kaleidoscope_test.sh"],
    data = [
        "kaleidoscope_test.out",
        ":kaleidoscope",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "kaleidoscope",
    srcs = ["kaleidoscope.cpp"],
    deps = ["@org_llvm_darwin",
    ],
)

sh_test(
    name = "kaleidoscope_test",
    data = [ ":kaleidoscope", "kaleidoscope_test.out" ],
    srcs = [ "kaleidoscope_test.sh" ],
)

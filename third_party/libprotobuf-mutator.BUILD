cc_library(
    name = "libprotobuf-mutator",
    srcs = glob(
        [
            "src/**/*.cc",
            "src/**/*.h",
            "port/protobuf.h",
        ],
        exclude = ["**/*_test.cc"],
    ),
    hdrs = ["src/libfuzzer/libfuzzer_macro.h"],
    includes = ["src/"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:protobuf"],
)

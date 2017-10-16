package(default_visibility = ["//visibility:public"])

cc_library(
    name = "hello-greet",
    srcs = ["hello-greet.cc"],
    hdrs = ["hello-greet.h"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "hello-world",
    srcs = ["hello-world.cc"],
    deps = [
        ":hello-greet",
        "//common",
        "//ast/desugar",
        #        "//lib:hello-time",
    ],
)

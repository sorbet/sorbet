proto_library(
    name = "proto",
    srcs = glob(["**/*.proto"]),
)

cc_proto_library(
    name = "pay-server",
    visibility = ["//visibility:public"],
    deps = [":proto"],
)

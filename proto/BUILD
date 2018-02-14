proto_library(
    name = "proto_proto",
    srcs = glob(["**/*.proto"]),
)

cc_proto_library(
    name = "proto",
    visibility = ["//visibility:public"],
    deps = [":proto_proto"],
)

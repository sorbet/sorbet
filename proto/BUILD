proto_library(
    name = "proto_proto",
    srcs = glob(["**/*.proto"]),
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

cc_proto_library(
    name = "proto",
    visibility = ["//visibility:public"],
    deps = [":proto_proto"],
)

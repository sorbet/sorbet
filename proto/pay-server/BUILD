load("@com_google_protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@com_google_protobuf//bazel:proto_library.bzl", "proto_library")

proto_library(
    name = "pay-server_proto",
    srcs = glob(["**/*.proto"]),
)

cc_proto_library(
    name = "pay-server",
    visibility = ["//visibility:public"],
    deps = [":pay-server_proto"],
)

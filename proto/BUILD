load("@com_google_protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@com_google_protobuf//bazel:proto_library.bzl", "proto_library")

filegroup(
    name = "protos",
    srcs = glob(["**/*.proto"]),
    visibility = ["//visibility:public"],
)

proto_library(
    name = "protos_proto",
    srcs = [":protos"],
)

cc_proto_library(
    name = "proto",
    visibility = ["//visibility:public"],
    deps = [":protos_proto"],
)

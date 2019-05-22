load("@com_google_protobuf//:protobuf.bzl", "cc_proto_library")

filegroup(
    name = "protos",
    srcs = glob(["**/*.proto"]),
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "proto",
    srcs = glob(["**/*.proto"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

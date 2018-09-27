load("@com_google_protobuf//:protobuf.bzl", "cc_proto_library")

cc_proto_library(
    name = "pay-server",
    srcs = glob(["**/*.proto"]),
    # has to come last: https://github.com/bazelbuild/bazel/issues/5924
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

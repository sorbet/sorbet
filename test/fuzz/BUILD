cc_binary(
    name = "fuzz_dash_e",
    srcs = [
        "fuzz_dash_e.cc",
    ],
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//main/pipeline",
        "//payload:light",
    ],
)

cc_binary(
    name = "fuzz_hover",
    srcs = [
        "fuzz_hover.cc",
    ],
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "proto",
        "//common",
        "//main:realmain",
        "//payload/binary:some",
        "//payload/text:empty",
        "@cxxopts",
        "@libprotobuf-mutator",
    ],
)

load("@com_google_protobuf//:protobuf.bzl", "cc_proto_library")

cc_proto_library(
    name = "proto",
    srcs = glob(["**/*.proto"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
)

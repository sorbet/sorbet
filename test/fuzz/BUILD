cc_binary(
    name = "fuzz_dash_e",
    srcs = ["empty.cc"],
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "fuzz_dash_e_impl",
    ],
)

cc_library(
    name = "fuzz_dash_e_impl",
    srcs = [
        "fuzz_dash_e.cc",
    ],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//main/pipeline",
        "//payload",
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

cc_test(
    name = "fuzz_doc_symbols",
    srcs = [
        "fuzz_doc_symbols.cc",
    ],
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//core",
        "//main/lsp",
        "//payload",
        "//test/helpers",
    ],
)

load("@com_google_protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@com_google_protobuf//bazel:proto_library.bzl", "proto_library")

proto_library(
    name = "proto_proto",
    srcs = glob(["**/*.proto"]),
)

cc_proto_library(
    name = "proto",
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [":proto_proto"],
)

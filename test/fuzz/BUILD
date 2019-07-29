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
        "//main/pipeline",
        "//main/lsp",
        "//payload",
        "//test/helpers",
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

cc_library(
    name = "payload",  # the actual payload used in release builds
    srcs = ["otherwise_compdb_bugs_out.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//payload:interface",
        "//payload/binary:some",
        "//payload/text:empty",
    ],
)

cc_library(
    name = "interface",  # the interface that defines to operate on a payload but does not actually add it
    srcs = ["payload.cc"],
    hdrs = ["payload.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/kvstore",
        "//core",
        "//core/serialize",
        "//main/options",
        "//main/pipeline",
        "//payload/binary",
        "//payload/text",
    ],
)

cc_library(
    name = "text",  # release payload in text form
    srcs = ["otherwise_compdb_bugs_out.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//payload:interface",
        "//payload/binary:empty",
        "//payload/text:some",
    ],
)

cc_library(
    name = "light",  # payload used in tests and fuzzers
    srcs = ["otherwise_compdb_bugs_out.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//payload:interface",
        "//payload/binary:light",
        "//payload/text:empty",
    ],
)

cc_library(
    name = "light_text",  # payload used in tests and fuzzers
    srcs = ["otherwise_compdb_bugs_out.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//payload:interface",
        "//payload/binary:empty",
        "//payload/text:light",
    ],
)

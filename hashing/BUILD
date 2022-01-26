cc_library(
    name = "hashing",
    srcs = [
        "hashing.cc",
    ],
    hdrs = ["hashing.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast/substitute",
        "//ast/treemap",
        "//common/concurrency",
        "//core",
        "//main/options",
        "//main/pipeline",
        "@rapidjson",
    ],
)

cc_library(
    name = "hashing-orig",
    srcs = [
        "hashing.cc",
    ],
    hdrs = ["hashing.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast/substitute",
        "//ast/treemap",
        "//common/concurrency",
        "//core",
        "//main/options",
        "//main/pipeline:pipeline-orig",
        "@rapidjson",
    ],
)

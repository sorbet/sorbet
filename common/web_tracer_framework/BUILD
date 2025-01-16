cc_library(
    name = "tracing",
    srcs = ["tracing.cc"],
    hdrs = [
        "tracing.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
        "//sorbet_version",
        "@rapidjson",
    ],
)

cc_test(
    name = "tracing_test",
    size = "small",
    srcs = glob(["test/*.cc"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":tracing",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

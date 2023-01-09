cc_library(
    name = "serialize",
    srcs = [
        "pickler.h",
        "serialize.cc",
    ],
    hdrs = ["serialize.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
        "@com_google_absl//absl/types:span",
        "@lz4",
    ],
)

cc_test(
    name = "serialize_test",
    size = "small",
    srcs = glob(["test/*.cc"]) + ["pickler.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":serialize",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

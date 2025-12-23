cc_library(
    name = "data",
    srcs = [
        "definitions.cc",
        "msgpack.cc",
        "msgpack_lite.cc",
    ],
    hdrs = [
        "definitions.h",
        "msgpack.h",
        "version.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//core",
        "@mpack",
    ],
)

cc_test(
    name = "data_test",
    size = "small",
    srcs = glob(["test/*.cc"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":data",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

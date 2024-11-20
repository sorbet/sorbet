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
        "@com_github_ludocode_mpack",
    ],
)

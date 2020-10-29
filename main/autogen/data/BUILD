cc_library(
    name = "data",
    srcs = [
        "definitions.cc",
        "msgpack.cc",
    ],
    hdrs = [
        "definitions.h",
        "msgpack.h",
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

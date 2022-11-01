cc_library(
    name = "exception",
    srcs = ["Exception.h"],
    hdrs = ["Exception.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/os",
        "@com_google_absl//absl/debugging:symbolize",
        "@spdlog",
    ],
)

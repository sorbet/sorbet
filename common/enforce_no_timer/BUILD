cc_library(
    name = "enforce_no_timer",
    srcs = ["EnforceNoTimer.h"],
    hdrs = ["EnforceNoTimer.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/exception",
        "//common/os",
        "//sorbet_version",
    ],
)

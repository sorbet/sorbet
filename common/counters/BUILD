cc_library(
    name = "counters",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "Counters.h",
        "Counters_impl.h",
    ],
    linkopts = select({
        "@platforms//os:linux": ["-lm"],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common/exception",
        "//common/sort",
        "//common/strings",
        "//sorbet_version",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@spdlog",
    ],
)

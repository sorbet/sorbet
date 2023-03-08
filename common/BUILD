cc_library(
    name = "common",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "FileOps.h",
        "FileSystem.h",
        "JSON.h",
        "Levenstein.h",
        "UIntSet.h",
        "UIntSetForEach.h",
        "common.h",
        "typecase.h",
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
        "//common/concurrency",
        "//common/counters",
        "//common/enforce_no_timer",
        "//common/exception",
        "//common/os",
        "//common/sort",
        "//common/strings",
        "//common/timers",
        "//sorbet_version",
        "//third_party/progressbar",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/debugging:symbolize",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@spdlog",
    ],
)

cc_test(
    name = "common_test",
    size = "small",
    srcs = glob(["test/*.cc"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":common",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

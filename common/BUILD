cc_library(
    name = "common",
    srcs = glob(
        [
            "*.cc",
            "*.h",
            "os/**/*.cc",
            "os/**/*.h",
        ],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "common.h",
        "typecase.h",
    ],
    linkopts = select({
        "//tools/config:linux": ["-lm"],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/strings",
        "@progressbar",
        "@skarupke_maps",
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
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

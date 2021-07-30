cc_library(
    name = "common",
    srcs = [
        "os/os.h",
        "os/os.cc",
    ] + glob(
        [
            "*.cc",
            "*.h",
        ],
        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ) + select({
        "//tools/config:darwin": ["os/mac.cc"],
        "//tools/config:webasm": ["os/emscripten.cc"],
        "//conditions:default": ["os/linux.cc"],
    }),
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
    defines = select({
        "//tools/config:webasm": [],
        "//conditions:default": ["USE_STL_FILESYSTEM"],
    }),
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
        "//sorbet_version",
        "//third_party/progressbar",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/debugging:symbolize",
        "@com_google_absl//absl/strings",
        "@pdqsort",
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
        "@doctest",
        "@doctest//:doctest_main",
    ],
)

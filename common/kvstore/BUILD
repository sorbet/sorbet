cc_library(
    name = "kvstore",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "KeyValueStore.h",
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
        "//common",
        "@com_google_absl//absl/strings",
        "@lmdb",
    ],
)

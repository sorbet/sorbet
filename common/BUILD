cc_library(
    name = "common",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.h",
        ],

        # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
        exclude = ["flycheck_*"],
    ),
    hdrs = [
        "common.h",
        "typecase.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/container:inlined_vector",
        "@spdlog//:spdlog",
    ],
)

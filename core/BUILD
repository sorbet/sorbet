cc_library(
    name = "core",
    srcs = glob(
        [
            "*.cc",
            "*.h",
            "types/*.cc",
            "lsp/*.cc",
        ],
        exclude = [
            # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
            "**/flycheck_*",
        ],
    ) + [
        "Names_gen.h",
        "Names_gen.cc",
    ],
    hdrs = [
        "core.h",
    ] + glob(["errors/*.h"]) + glob(["lsp/*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@com_google_absl//absl/strings",
        "@rang",
    ],
)

cc_test(
    name = "core_test",
    size = "small",
    srcs = glob(
        ["test/*.cc"],
        exclude = [
            # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
            "**/flycheck_*",
        ],
    ),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":core",
        "@gtest",
        "@gtest//:gtest_main",
    ],
)

cc_binary(
    name = "generate_names",
    srcs = [
        "tools/generate_names.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
    ],
)

genrule(
    name = "generate_names_h",
    outs = [
        "Names_gen.h",
        "Names_gen.cc",
    ],
    cmd = "$(location :generate_names) $(location Names_gen.h) $(location Names_gen.cc)",
    tools = [":generate_names"],
)

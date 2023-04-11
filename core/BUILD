cc_library(
    name = "core",
    srcs = glob(
        [
            "*.cc",
            "*.h",
            "errors/*.cc",
            "types/*.cc",
            "lsp/*.cc",
            "packages/*.cc",
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
        "NameSubstitution.h",
        "core.h",
    ] + glob([
        "errors/*.h",
        "lsp/*.h",
        "packages/*.h",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//common/concurrency",
        "//core/hashing",
        "//main/pipeline/semantic_extension:interface",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:span",
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
        "@doctest//doctest",
        "@doctest//doctest:main",
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
    cmd = "$(location :generate_names) $(location Names_gen.h) $(location Names_gen.cc) && \
             $(location //tools:clang-format) -i $(location Names_gen.h) $(location Names_gen.cc)",
    tools = [
        ":generate_names",
        "//tools:clang-format",
    ],
)

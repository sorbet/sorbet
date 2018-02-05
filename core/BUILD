NAME_PHASES = [
    "core",
    "parser",
    "desugar",
    "dsl",
    "namer",
    "resolver",
    "cfg",
    "infer",
]

NAME_HEADERS = ["Names/%s.h" % (phase,) for phase in NAME_PHASES]

cc_library(
    name = "core",
    srcs = glob(
        [
            "*.cc",
            "*.h",
            "types/*.cc",
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
    ] + glob(["errors/*.h"]) + NAME_HEADERS,
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "core_test",
    size = "small",
    srcs = glob(["test/*.cc"]),
    linkstatic = select({
        "//tools/config:fastbuild": 0,
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
    copts = ["--std=c++14"],
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
    ] + NAME_HEADERS,
    cmd = "$(location :generate_names) $(location Names_gen.h) $(location Names_gen.cc) " +
          " ".join(["--%s=$(location %s)" % e for e in zip(NAME_PHASES, NAME_HEADERS)]),
    tools = [":generate_names"],
)

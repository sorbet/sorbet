# NOTE: If a target depends on this, that target **must** cargo cult the linkopts
# from `rubyfmt_test`! Otherwise, it will not compile+link properly.
cc_library(
    name = "rubyfmt",
    srcs = select({
        "//tools/config:webasm": ["rubyfmt-emscripten.cc"],
        "//conditions:default": ["rubyfmt.cc"],
    }),
    hdrs = [
        "rubyfmt.h",
    ],
    features = ["fully_static_link"],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = select({
        "//tools/config:webasm": [],
        "//conditions:default": [
            "@ruby_2_6//:rubyfmt-static-deps",
            "@rubyfmt//:librubyfmt",
            "@rubyfmt//:rubyfmt_macro_wrappers",
            "@com_google_absl//absl/synchronization",
        ],
    }),
    alwayslink = True,
)

cc_test(
    name = "rubyfmt_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:darwin": ["-framework Foundation"],
        "//tools/config:webasm": [],
        "//conditions:default": [
            "-lcrypt",
            "-lrt",
            "-lz",
            "-lgmp",
        ],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//experimental/rubyfmt",
        "@doctest",
        "@doctest//:doctest_main",
    ],
)

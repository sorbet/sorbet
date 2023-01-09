cc_library(
    name = "options",
    srcs = [
        "ConfigParser.cc",
        "ConfigParser.h",
        "options.cc",
    ],
    hdrs = ["options.h"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//core",
        "//sorbet_version",
        "//third_party/licenses",
        "@cxxopts",
        "@spdlog",
        "@yaml_cpp",
    ],
)

cc_test(
    name = "options_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//core",
        "//main/options",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

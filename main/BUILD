cc_binary(
    name = "ruby-typer",
    srcs = [
        "main.cc",
        "state-payload.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "realmain",
        "state-payload",
    ],
)

cc_library(
    name = "state-payload",
    srcs = [
        "state-payload.cc",
    ],
    hdrs = [
        "payload.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
    ],
)

cc_library(
    name = "no-state-payload",
    srcs = [
        "no_payload.cc",
    ],
    hdrs = [
        "payload.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
    ],
)

cc_binary(
    name = "ruby-typer-orig",
    srcs = [
        "main.cc",
    ],
    copts = ["--std=c++14"],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "no-state-payload",
        "realmain",
    ],
)

cc_binary(
    name = "gen_state_payload",
    srcs = [
        "tools/gen_state_payload.cc",
    ],
    copts = ["--std=c++14"],
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

genrule(
    name = "generate-state-payload-cc",
    srcs = ["state-payload-raw"],
    outs = [
        "state-payload.cc",
    ],
    cmd = "$(location :gen_state_payload) $(location state-payload-raw) $(location state-payload.cc)",
    tools = [":gen_state_payload"],
)

genrule(
    name = "generate-state-payload-raw",
    srcs = [],
    outs = [
        "state-payload-raw",
    ],
    cmd = "$(location :ruby-typer-orig) --store-state $(location state-payload-raw) -e \"1+1\"",
    tools = [":ruby-typer-orig"],
)

cc_library(
    name = "realmain",
    srcs = [
        "payload.h",
        "realmain.cc",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//ast/desugar",
        "//cfg",
        "//core",
        "//core/serialize",
        "//infer",
        "//namer",
        "//parser",
        "//rbi:payload",
        "@cxxopts//:cxxopts",
    ],
)

cc_test(
    name = "stdlib_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//rbi:stdlib",
        "//test:gerald",
    ],
    linkstatic = select({
        "//tools/config:fastbuild": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "no-state-payload",
        "realmain",
        "@cxxopts//:cxxopts",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)

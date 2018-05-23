cc_library(
    name = "binary",
    hdrs = [
        "binary.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
    ],
)

cc_library(
    name = "empty",
    srcs = [
        "empty.cc",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "binary",
    ],
)

cc_library(
    name = "some",
    srcs = [
        "binary.cc",
    ],
    hdrs = [
        "binary.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
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
        "binary.cc",
    ],
    cmd = "$(location :gen_state_payload) $(location state-payload-raw) $(location binary.cc)",
    tools = [":gen_state_payload"],
)

genrule(
    name = "generate-state-payload-raw",
    srcs = [],
    outs = [
        "state-payload-raw",
    ],
    cmd = "$(location //main:sorbet-orig) --store-state $(location state-payload-raw) -e 1",
    tools = ["//main:sorbet-orig"],
)

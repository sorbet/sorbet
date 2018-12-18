cc_library(
    name = "binary",
    hdrs = [
        "binary.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
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
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
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
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
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
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

genrule(
    name = "generate-state-payload-cc",
    srcs = ["state-payload-raw"],
    outs = [
        "binary.cc",
    ],
    cmd = "$(location :gen_state_payload) $(location state-payload-raw) $(location binary.cc) && \
             $(location //tools:clang-format) -i $(location binary.cc)",
    tools = [
        ":gen_state_payload",
        "//tools:clang-format",
    ],
)

genrule(
    name = "generate-state-payload-raw",
    srcs = [],
    outs = [
        "state-payload-raw",
    ],
    cmd = "$(location //main:sorbet-orig) --silence-dev-message  --store-state $(location state-payload-raw) --no-error-count",
    tools = ["//main:sorbet-orig"],
)

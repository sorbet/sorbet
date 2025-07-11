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
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "binary",
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
    srcs = [
        "state-payload-raw-symbol-table",
        "state-payload-raw-name-table",
        "state-payload-raw-file-table",
    ],
    outs = [
        "binary.cc",
    ],
    cmd = """
    $(location :gen_state_payload) \
        $(location state-payload-raw-symbol-table) \
        $(location state-payload-raw-name-table) \
        $(location state-payload-raw-file-table) \
        $(location binary.cc)
    """,
    tools = [
        ":gen_state_payload",
    ],
)

genrule(
    name = "generate-state-payload-raw",
    srcs = [],
    outs = [
        "state-payload-raw-symbol-table",
        "state-payload-raw-name-table",
        "state-payload-raw-file-table",
    ],
    cmd = """
    $(location //main:sorbet-orig) \
        --silence-dev-message \
        --no-error-count \
        --store-state $(location state-payload-raw-symbol-table),$(location state-payload-raw-name-table),$(location state-payload-raw-file-table)
    """,
    tools = ["//main:sorbet-orig"],
)

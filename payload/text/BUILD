cc_binary(
    name = "generate_payload",
    srcs = [
        "tools/generate_payload.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "//version",
    ],
)

cc_library(
    name = "text",
    hdrs = [
        "text.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [],
)

cc_library(
    name = "some",
    srcs = [
        "text.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = ["text"],
)

cc_library(
    name = "light",
    srcs = [
        "light.cc",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = ["text"],
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
    deps = ["text"],
)

genrule(
    name = "generate_text_payload_cc",
    srcs = ["//rbi"],
    outs = [
        "text.cc",
    ],
    cmd = "$(location :generate_payload) $(location text.cc) $(locations //rbi)",
    tools = [
        ":generate_payload",
    ],
)

genrule(
    name = "generate_light_payload_cc",
    srcs = ["//rbi:light"],
    outs = [
        "light.cc",
    ],
    cmd = "$(location :generate_payload) $(location light.cc) $(locations //rbi:light)",
    tools = [
        ":generate_payload",
    ],
)

genrule(
    name = "generate_text_empty_payload_cc",
    outs = [
        "empty.cc",
    ],
    cmd = "$(location :generate_payload) $(location empty.cc)",
    tools = [
        ":generate_payload",
    ],
)

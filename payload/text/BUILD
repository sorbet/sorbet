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
    name = "test_bootstrap",
    srcs = [
        "test_bootstrap.cc",
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
    cmd = "$(location :generate_payload) $(location text.cc) $(locations //rbi) && \
    $(location //tools:clang-format) -i $(location text.cc)",
    tools = [
        ":generate_payload",
        "//tools:clang-format",
    ],
)

genrule(
    name = "generate_test_bootstrap_payload_cc",
    srcs = ["//rbi:test_bootstrap"],
    outs = [
        "test_bootstrap.cc",
    ],
    cmd = "$(location :generate_payload) $(location test_bootstrap.cc) $(locations //rbi:test_bootstrap) && \
    $(location //tools:clang-format) -i $(location test_bootstrap.cc)",
    tools = [
        ":generate_payload",
        "//tools:clang-format",
    ],
)

genrule(
    name = "generate_text_empty_payload_cc",
    outs = [
        "empty.cc",
    ],
    cmd = "$(location :generate_payload) $(location empty.cc)&& \
       $(location //tools:clang-format) -i $(location empty.cc)",
    tools = [
        ":generate_payload",
        "//tools:clang-format",
    ],
)

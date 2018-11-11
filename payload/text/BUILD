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
        "text.h",
    ],
    cmd = "$(location :generate_payload) $(location text.h) $(location text.cc) $(locations //rbi) && \
    $(location //tools:clang-format) -i $(location text.h) $(location text.cc)",
    tools = [
        ":generate_payload",
        "//tools:clang-format",
    ],
)

genrule(
    name = "generate__text_empty_payload_cc",
    outs = [
        "empty.cc",
        "empty.h",
    ],
    cmd = "$(location :generate_payload) $(location empty.h) $(location empty.cc)&& \
       $(location //tools:clang-format) -i $(location empty.h) $(location empty.cc)",
    tools = [
        ":generate_payload",
        "//tools:clang-format",
    ],
)

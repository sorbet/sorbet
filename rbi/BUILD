filegroup(
    name = "stdlib",
    srcs = [
        "stdlib.rbi",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "generate_payload",
    srcs = [
        "tools/generate_payload.cc",
    ],
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

cc_library(
    name = "payload",
    srcs = [
        "payload.cc",
        "payload.h",
    ],
    hdrs = [
        "payload.h",
    ],
    visibility = ["//visibility:public"],
    deps = [],
)

genrule(
    name = "generate_payload_cc",
    srcs = ["stdlib"],
    outs = [
        "payload.cc",
        "payload.h",
    ],
    cmd = "$(location :generate_payload) $(location payload.h) $(location payload.cc) $(locations stdlib)",
    tools = [":generate_payload"],
)

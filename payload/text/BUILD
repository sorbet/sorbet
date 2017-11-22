cc_binary(
    name = "generate_payload",
    srcs = [
        "tools/generate_payload.cc",
    ],
    copts = ["--std=c++14"],
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

cc_library(
    name = "text",
    srcs = [
        "text.cc",
        "text.h",
    ],
    hdrs = [
        "text.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [],
)

genrule(
    name = "generate_payload_cc",
    srcs = ["//rbi:stdlib"],
    outs = [
        "text.cc",
        "text.h",
    ],
    cmd = "$(location :generate_payload) $(location text.h) $(location text.cc) $(locations //rbi:stdlib)",
    tools = [":generate_payload"],
)

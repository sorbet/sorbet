licenses(["notice"])

cc_library(
    name = "licences",
    srcs = [
        "licences.cc",
    ],
    hdrs = [
        "licences.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "licence_files",
    srcs = glob(
        ["*.txt"],
    ),
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "generate_licences",
    srcs = ["tools/generate_licences.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

genrule(
    name = "generate_text_payload_cc",
    srcs = ["licence_files"],
    outs = [
        "licences.cc",
    ],
    cmd = "$(location :generate_licences) $(location licences.cc) $(locations licence_files) && \
    $(location //tools:clang-format) -i $(location licences.cc)",
    tools = [
        ":generate_licences",
        "//tools:clang-format",
    ],
)

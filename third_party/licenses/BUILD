licenses(["notice"])

cc_library(
    name = "licenses",
    srcs = [
        "licenses.cc",
    ],
    hdrs = [
        "licenses.h",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
)

filegroup(
    name = "license_files",
    srcs = glob(
        ["*.txt"],
    ),
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "generate_licenses",
    srcs = ["tools/generate_licenses.cc"],
    linkstatic = True,
    visibility = ["//tools:__pkg__"],
    deps = ["//common"],
)

genrule(
    name = "generate_text_payload_cc",
    srcs = ["license_files"],
    outs = [
        "licenses.cc",
    ],
    cmd = "$(location :generate_licenses) $(location licenses.cc) $(locations license_files) && \
    $(location //tools:clang-format) -i $(location licenses.cc)",
    tools = [
        ":generate_licenses",
        "//tools:clang-format",
    ],
)

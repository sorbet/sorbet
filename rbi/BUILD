load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file")

filegroup(
    name = "rbi",
    srcs = glob(
        ["**/*.rbi"],
    ),
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "generate_procs",
    srcs = ["tools/generate_procs.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
)

genrule(
    name = "generate_procs_rbi",
    outs = ["procs.rbi.gen"],
    cmd = "$(location :generate_procs) $@",
    tools = [":generate_procs"],
)

write_source_file(
    name = "update_procs_rbi",
    in_file = ":generate_procs_rbi",
    out_file = "procs.rbi",
)

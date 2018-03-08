filegroup(
    name = "rbi",
    srcs = glob([
        "**/*.rbi",
    ]) + [":generate_procs_rbi"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "generate_procs",
    srcs = ["tools/generate_procs.cc"],
    visibility = ["//tools:__pkg__"],
)

genrule(
    name = "generate_procs_rbi",
    outs = ["procs.rbi"],
    cmd = "$(location :generate_procs) $@",
    tools = [":generate_procs"],
)

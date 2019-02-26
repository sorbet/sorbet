filegroup(
    name = "rbi",
    srcs = glob(
        ["**/*.rbi"],
        exclude = ["test_bootstrap.rbi"],
    ) + [":generate_procs_rbi"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "test_bootstrap",
    srcs =
        [
            "test_bootstrap.rbi",
            ":generate_procs_rbi",
        ],
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
    outs = ["procs.rbi"],
    cmd = "$(location :generate_procs) $@",
    tools = [":generate_procs"],
)

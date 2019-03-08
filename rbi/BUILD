filegroup(
    name = "rbi",
    srcs = glob(
        ["**/*.rbi"],
        exclude = ["light.rbi"],
    ) + [":generate_procs_rbi"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "light",
    srcs =
        [
            "light.rbi",
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

filegroup(
    name = "rbi",
    srcs = glob(
        [
            "sorbet-typed/lib/core/all/**/*.rbi",
            "sorbet-typed/lib/stdlib/all/**/*.rbi",
            "sorbet-typed/lib/sorbet/all/**/*.rbi",
            "sorbet-typed/lib/bundler/all/**/*.rbi",
            "sorbet-typed/lib/chalk/all/**/*.rbi",
            "sorbet-typed/lib/configatron/all/**/*.rbi",
            "sorbet-typed/lib/didyoumean/all/**/*.rbi",
            "sorbet-typed/lib/msgpack/all/**/*.rbi",
        ],
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

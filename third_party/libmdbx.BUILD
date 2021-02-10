cc_library(
    name = "libmdbx",
    srcs = [
        "mdbx.c",
    ],
    hdrs = [
        "mdbx.h",
    ],
    copts = ["-Wno-missing-field-initializers"],
    defines = ["MDBX_BUILD_FLAGS=''"],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

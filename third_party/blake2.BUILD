# note: this isn't the blake2 used in prod builds
# this one is only used in webasm/aarch64 mode
# prod builds use libb2

cc_library(
    name = "com_github_blake2_blake2_ref",
    srcs = [
        "ref/blake2s-ref.c",
        "ref/blake2b-ref.c",
    ] + glob(["ref/*.h"]),
    hdrs = [
        "ref/blake2.h",
        "ref/blake2-impl.h",
    ],
    includes = [
        "ref",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "com_github_blake2_blake2_neon",
    srcs = [
        "neon/blake2b-neon.c",
        "neon/blake2s-neon.c",
    ] + glob(["neon/*.h"]),
    hdrs = [
        "neon/blake2.h",
        "neon/blake2-impl.h",
    ],
    includes = [
        "neon",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

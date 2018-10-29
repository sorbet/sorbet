# note: this isn't the blake2 used in prod builds
# this one is only used in webasm mode
# prod builds use libb2

cc_library(
    name = "com_github_blake2_blake2",
    srcs = ["ref/blake2s-ref.c", "ref/blake2b-ref.c"] + glob(["src/*.h"]),
    hdrs = [
        "ref/blake2.h",
        "ref/blake2-impl.h"
    ],
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
    }),
)

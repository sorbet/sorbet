cc_library(
    name = "xxhash",
    srcs = glob(["xxhash.c"]),
    hdrs = glob(["xxhash.h"]),
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

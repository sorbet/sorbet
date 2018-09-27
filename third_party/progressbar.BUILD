cc_library(
    name = "progressbar",
    srcs = glob(["src/*.c"]),
    hdrs = glob([
        "progressbar/*.h"
    ]),
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)

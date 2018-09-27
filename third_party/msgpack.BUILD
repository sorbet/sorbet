cc_library(
    name = "com_github_msgpack_msgpack",
    srcs = glob(["include/msgpack/**/*.hpp", "include/msgpack/**/*.h"]),
    hdrs = [
        "include/msgpack.hpp",
    ],
    includes = [
        "include",
    ],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)

cc_library(
    name = "lmdb",
    srcs = [
        "libraries/liblmdb/lmdb.h",
        "libraries/liblmdb/mdb.c",
        "libraries/liblmdb/midl.c",
        "libraries/liblmdb/midl.h",
    ],
    hdrs = [
        "libraries/liblmdb/lmdb.h",
        "libraries/liblmdb/midl.h",
    ],
    copts = ["-Wno-implicit-fallthrough"],
    includes = [
        "libraries/liblmdb/",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "lmdb",
    srcs = [
       "libraries/liblmdb/mdb.c",
       "libraries/liblmdb/lmdb.h",
       "libraries/liblmdb/midl.h",
       "libraries/liblmdb/midl.c",
       ],
    hdrs = [
        "libraries/liblmdb/midl.h",
        "libraries/liblmdb/lmdb.h",
        ],
    visibility = ["//visibility:public"],
    includes = [
            "libraries/liblmdb/",
        ],
        linkstatic = select({
                "@com_stripe_ruby_typer//tools/config:linkshared": 0,
                "//conditions:default": 1,
            }),
)

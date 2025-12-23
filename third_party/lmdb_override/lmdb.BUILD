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
    copts = [
        "-Wno-implicit-fallthrough",
        "-Wno-unused-but-set-variable",
    ],
    includes = [
        "libraries/liblmdb/",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
)

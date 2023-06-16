filegroup(
    name = "headers",
    srcs = [
        "sqlite3.h",
        "sqlite3ext.h",
    ],
    visibility = ["//visibility:public"],
)

include_sqlite = [
    ".",
]

sqlite_copts = [
    "-Wno-implicit-fallthrough",
    "-Wno-misleading-indentation",
    "-DSQLITE_THREADSAFE=0",
    "-DQLITE_DEFAULT_MEMSTATUS=0",
    "-DSQLITE_LIKE_DOESNT_MATCH_BLOBS",
    "-DSQLITE_OMIT_DEPRECATED",
    "-DSQLITE_OMIT_SHARED_CACHE",
    "-DHAVE_USLEEP",
    "-DHAVE_UTIME",
    "-DSQLITE_BYTEORDER=1234",
    "-DSQLITE_DEFAULT_AUTOVACUUM=0",
    "-DSQLITE_DEFAULT_MMAP_SIZE=0",
    "-DSQLITE_CORE",
    "-DSQLITE_TEMP_STORE=3",
    "-DSQLITE_OMIT_LOAD_EXTENSION",
    "-DSQLITE_OMIT_RANDOMNESS",
]

cc_library(
    name = "sqlite",
    srcs = [
        "sqlite3.c",
        "sqlite3.h",
    ],
    hdrs = [":headers"],
    copts = sqlite_copts,
    includes = include_sqlite,
    visibility = ["//visibility:public"],
)

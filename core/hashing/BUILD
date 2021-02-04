cc_library(
    name = "hashing",
    srcs = glob(
        [
            "*.cc",
        ],
        exclude = [
            # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
            "**/flycheck_*",
        ],
    ),
    hdrs = [
        "hashing.h",
    ],
    copts = ["-Wno-implicit-fallthrough"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@xxhash",
    ],
)

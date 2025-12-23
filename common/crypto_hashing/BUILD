cc_library(
    name = "crypto_hashing",
    hdrs = glob(["*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = ["//common"] + select({
        "@platforms//cpu:x86_64": ["@libb2"],
        "@platforms//cpu:arm64": ["@blake2//:com_github_blake2_blake2_neon"],
        "//conditions:default": ["@blake2//:com_github_blake2_blake2_ref"],
    }),
)

cc_library(
    name = "crypto_hashing",
    hdrs = glob(["*.h"]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = ["//common"] + select({
        "//tools/config:webasm": ["@com_github_blake2_blake2"],
        "//tools/config:darwin_x86_64": ["@com_github_blake2_libb2"],
        "//tools/config:linux_x86_64": ["@com_github_blake2_libb2"],
        "//conditions:default": ["@com_github_blake2_blake2"],
    }),
)

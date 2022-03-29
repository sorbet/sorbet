cc_import(
    name = "ssl-import",
    shared_library = "lib/x86_64-linux-gnu/libssl.so",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "ssl",
    hdrs = glob(["include/openssl/**/*.h"]),
    includes = ["include/openssl"],
    visibility = ["//visibility:public"],
    deps = [":ssl-import"],
)

cc_import(
    name = "crypto-import",
    shared_library = "lib/x86_64-linux-gnu/libcrypto.so",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "crypto",
    hdrs = glob(["include/openssl/**/*.h"]),
    includes = ["include/openssl"],
    visibility = ["//visibility:public"],
    deps = [":crypto-import"],
)

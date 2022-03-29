cc_import(
    name = "ssl-import",
    shared_library = "lib/libssl.dylib",
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
    shared_library = "lib/libcrypto.dylib",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "crypto",
    hdrs = glob(["include/openssl/**/*.h"]),
    includes = ["include/openssl"],
    visibility = ["//visibility:public"],
    deps = [":crypto-import"],
)

cc_import(
    name = "ssl-import",
    shared_library = "lib/libssl.dylib",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "ssl",
    hdrs = glob(["include/openssl/**/*.h"]),
    # "include" is added to the includes as openssl is installed in /usr/local/opt/openssl on darwin.
    includes = ["include"],
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
    # "include" is added to the includes as openssl is installed in /usr/local/opt/openssl on darwin.
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":crypto-import"],
)

cc_library(
    name = "rbs_parser",
    srcs = glob(["src/**/*.c"]),
    hdrs = glob(["include/**/*.h"]),
    copts = [
        "-Wno-error=missing-field-initializers",
        "-Wno-error=implicit-fallthrough",
    ],
    includes = ["include"],
    linkstatic = True,
    visibility = ["//visibility:public"],
)

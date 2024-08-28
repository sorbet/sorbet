cc_library(
    name = "rbs_parser",
    srcs = glob(["src/**/*.c"]),
    hdrs = glob(["include/**/*.h"]),
    copts = [
        "-Iexternal/rbs_parser/include",
        "-Wno-error=missing-field-initializers",
        "-Wno-error=implicit-fallthrough",
    ],
    visibility = ["//visibility:public"],
)

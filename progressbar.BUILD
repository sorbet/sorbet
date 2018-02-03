cc_library(
    name = "progressbar",
    srcs = glob(["lib/*.c"]),
    hdrs = glob([
        "include/progressbar/*.h"
    ]),
    copts = ["-Iexternal/progressbar/include/progressbar/"],
    linkopts = ["-lcurses"],
    includes = ["include/"],
    visibility = ["//visibility:public"],
)

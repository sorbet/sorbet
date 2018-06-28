cc_library(
    name = "spdlog",
    srcs = [],
    hdrs = glob([
        "include/spdlog/**/*.h",
        "include/spdlog/**/*.cc",
    ]),
    copts = [
        "-Iexternal/spdlog/",
        "--std=c++14"
    ],
    includes = [
        "include/",
    ],
    linkopts = [
        "-lpthread"
    ],
    visibility = ["//visibility:public"],
)

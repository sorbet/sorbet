cc_library(
    name = "spdlog",
    srcs = glob([
        "src/**/*.cpp",
    ]),
    defines = [
        "SPDLOG_COMPILED_LIB",
    ],
    hdrs = glob([
        "include/spdlog/**/*.h",
    ]),
    copts = [
        "-Iexternal/spdlog/",
    ],
    includes = [
        "include/",
    ],
    linkopts = [
        "-lpthread",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
)

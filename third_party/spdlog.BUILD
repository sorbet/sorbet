cc_library(
    name = "spdlog",
    srcs = glob([
        "src/**/*.cpp",
    ]),
    defines = [
        # This define switches from using spdlog as a header-only library to a
        # compiled library (*.a / *.so). When this change was implemented, it
        # dropped the time to recompile a change to common/common.h (included
        # ~everywhere) from ~5 minutes to ~3 minutes (May 2020).
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

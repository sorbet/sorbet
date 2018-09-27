cc_library(
    name = "yaml_cpp",
    srcs = glob([
        "src/*.cpp",
        "src/*.h",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    copts = [
        "-Iexternal/yaml_cpp/include",
    ],
    includes = [
        "include/",
    ],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)

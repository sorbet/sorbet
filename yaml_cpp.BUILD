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
        "--std=c++14"
    ],
    includes = [
        "include/",
    ],
    visibility = ["//visibility:public"],
)

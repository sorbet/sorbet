cc_binary(
    name = "fuzz_dash_e",
    srcs = ["empty.cc"],
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "fuzz_dash_e_impl",
    ],
)

cc_library(
    name = "fuzz_dash_e_impl",
    srcs = [
        "fuzz_dash_e.cc",
    ],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//main/pipeline",
        "//payload",
    ],
)

cc_test(
    name = "fuzz_doc_symbols",
    srcs = [
        "fuzz_doc_symbols.cc",
    ],
    tags = ["manual"],
    visibility = ["//tools:__pkg__"],
    deps = [
        "//core",
        "//main/lsp",
        "//payload",
        "//test/helpers",
    ],
)

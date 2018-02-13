package(default_visibility = ["//tools/cpp/ci:__pkg__"])

WRAPPERS = [
    "clang",
    "clang-cpp",
    "llvm-ar",
    "llvm-nm",
    "llvm-dwp",
    "llvm-objdump",
]

[
    filegroup(
        name = shim,
        srcs = [
            "shim-{}".format(shim),
            "@clang_5_0_0_linux//:bin/{}".format(shim),
        ],
    )
    for shim in WRAPPERS
]

filegroup(
    name = "wrappers",
    srcs = [":{}".format(w) for w in WRAPPERS],
)

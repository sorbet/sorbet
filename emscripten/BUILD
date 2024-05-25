load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

BASE_LINKOPTS = [
    "-s",
    "MODULARIZE=1",
    "-s",
    "EXPORT_NAME=Sorbet",
    "-s",
    "EXPORTED_RUNTIME_METHODS=ccall,cwrap,addFunction,UTF8ToString",
    "-s",
    "INITIAL_MEMORY=67108864",
    # This let's us allocate 1 JavaScript function (the callback for
    # JSON-RPC responses) as a function pointer for use in C++
    # (see addFunction above).
    "-s",
    "ALLOW_TABLE_GROWTH=1",
]

cc_binary(
    name = "sorbet-wasm",
    srcs = ["main.cc"],
    linkopts = BASE_LINKOPTS,
    # This target won't build successfully on its own because of missing
    # emscripten headers, etc. Therefore, we hide it from wildcards.
    tags = ["manual"],
    visibility = ["//visibility:public"],
    deps = [
        "//main:realmain",
        "//main/pipeline/semantic_extension:none",
        "//payload/binary:some",
        "//payload/text:empty",
    ],
)

wasm_cc_binary(
    name = "sorbet-wasm.d",
    cc_target = ":sorbet-wasm",
    visibility = ["//visibility:public"],
)

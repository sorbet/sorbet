load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

BASE_LINKOPTS = [
    "-s",
    "MODULARIZE=1",
    "-s",
    "EXPORT_NAME=Sorbet",
    "-s",
    "EXPORTED_RUNTIME_METHODS=ccall,cwrap,addFunction,Pointer_stringify",
    "-s",
    "INITIAL_MEMORY=67108864",
    # This let's us allocate 1 JavaScript function (the callback for
    # JSON-RPC responses) as a function pointer for use in C++
    # (see addFunction above).
    "-s",
    "ALLOW_TABLE_GROWTH=1",
]

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    linkopts = BASE_LINKOPTS,
    deps = [
        "//main:realmain",
        "//main/pipeline/semantic_extension:none",
        "//payload/binary:some",
        "//payload/text:empty",
    ],
    # This target won't build successfully on its own because of missing
    # emscripten headers, etc. Therefore, we hide it from wildcards.
    tags = ["manual"],
)

wasm_cc_binary(
    name = "sorbet-wasm",
    cc_target = ":main",
)

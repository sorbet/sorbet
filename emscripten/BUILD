cc_binary(
    name = "sorbet-wasm.tar",
    linkopts = select({
        "//tools/config:webasm": [
            "-s",
            "EXPORT_NAME=\"Sorbet\"",
            "-s",
            "MODULARIZE=1",
            "-s",
            "EXPORTED_FUNCTIONS=[\"_typecheck\",\"_lsp\"]",
            "-s",
            # - `addFunction` converts a JavaScript function to a C++ function
            #    pointer. It is used in the Sourcegraph extension
            #    https://sourcegraph.com/extensions/sourcegraph/lang-ruby
            #    to enable Sorbet to push JSON-RPC responses to JavaScript and
            #    avoid polling. This requires 1 reserved function pointer
            #    (see RESERVED_FUNCTION_POINTERS below).
            # - `Pointer_stringify` converts a C++ char* to a JavaScript string.
            "EXTRA_EXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\",\"addFunction\",\"Pointer_stringify\"]",
            "-s",
            "TOTAL_MEMORY=33554432",
            "-s",
            # This let's us allocate 1 JavaScript function (the callback for
            # JSON-RPC responses) as a function pointer for use in C++
            # (see addFunction above).
            "RESERVED_FUNCTION_POINTERS=1",
            "-s",
            "DISABLE_EXCEPTION_CATCHING=2",
        ],
        "//conditions:default": [],
    }),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//tools/config:webasm": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":main",
    ],
)

cc_library(
    name = "main",
    srcs = ["main.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//main:realmain",
        "//payload/binary:some",
        "//payload/text:empty",
    ],
)

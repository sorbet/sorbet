cc_binary(
    name = "sorbet-wasm.tar",
    linkopts = select({
        "//tools/config:webasm": [
            "-s",
            "EXPORT_NAME=\"Sorbet\"",
            "-s",
            "MODULARIZE=1",
            "-s",
            "EXPORTED_FUNCTIONS=[\"_typecheck\"]",
            "-s",
            "EXTRA_EXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\"]",
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

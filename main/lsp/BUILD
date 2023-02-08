cc_library(
    name = "lsp",
    srcs = glob([
        "*.cc",
        "requests/*.h",
        "requests/*.cc",
        "notifications/*.h",
        "notifications/*.cc",
        "watchman/*.cc",
    ]) + [
        ":lsp_messages",
        "AbstractRewriter.h",
        "DefLocSaver.h",
        "ErrorFlusherLSP.h",
        "ErrorReporter.h",
        "FieldFinder.h",
        "LSPFileUpdates.h",
        "LSPIndexer.h",
        "LSPTask.h",
        "LSPTypechecker.h",
        "LSPTypecheckerCoordinator.h",
        "LocalVarFinder.h",
        "LocalVarSaver.h",
        "NextMethodFinder.h",
        "QueryCollector.h",
        "ShowOperation.h",
        "UndoState.h",
        "json_enums.h",
        "lsp_messages_gen_helpers.h",
        "watchman/WatchmanProcess.h",
    ],
    hdrs = [
        "LSPConfiguration.h",
        "LSPInput.h",
        "LSPLoop.h",
        "LSPMessage.h",
        "LSPOutput.h",
        "LSPPreprocessor.h",
        "LSPQuery.h",
        "MoveMethod.h",
        "ConvertToSingletonClassMethod.h",
        "json_types.h",
        "wrapper.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//ast",
        "//common/kvstore",
        "//common/statsd",
        "//common/web_tracer_framework:tracing",
        "//core",
        "//core/proto",
        "//core/serialize",
        "//hashing",
        "//main/cache",
        "//main/options",
        "//main/pipeline",
        "//main/sig_finder",
        "//payload:interface",
        "//payload/binary",
        "//payload/text",
        "@com_google_absl//absl/synchronization",
        "@cpp_subprocess",
        "@rapidjson",
    ],
)

cc_binary(
    name = "generate_lsp_messages",
    srcs = glob([
        "tools/*.h",
        "tools/*.cc",
    ]),
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//common",
        "@rapidjson",
    ],
)

LSP_MESSAGES_SHARDS = [
    "lsp_messages_gen_1.cc",
    "lsp_messages_gen_2.cc",
    "lsp_messages_gen_3.cc",
    "lsp_messages_gen_4.cc",
    "lsp_messages_gen_5.cc",
]

filegroup(
    name = "lsp_messages",
    srcs = [
        "lsp_messages_gen.h",
        "lsp_messages_enums_gen.h",
        "lsp_messages_enums_gen.cc",
    ] + LSP_MESSAGES_SHARDS,
)

genrule(
    name = "generate_lsp_messages_h",
    outs = [
        "lsp_messages_enums_gen.h",
        "lsp_messages_enums_gen.cc",
        "lsp_messages_gen.h",
    ] + LSP_MESSAGES_SHARDS,
    cmd = """
    set -euo pipefail

    $(location :generate_lsp_messages) \
        $(location lsp_messages_enums_gen.h) \
        $(location lsp_messages_enums_gen.cc) \
        $(location lsp_messages_gen.h) \
        {shards}

    $(location //tools:clang-format) -i \
        $(location lsp_messages_enums_gen.h) \
        $(location lsp_messages_enums_gen.cc) \
        $(location lsp_messages_gen.h) \
        {shards}
    """.format(shards = " ".join(["$(location {})".format(file) for file in LSP_MESSAGES_SHARDS])),
    tools = [
        ":generate_lsp_messages",
        "//tools:clang-format",
    ],
)

cc_test(
    name = "error_reporter_test",
    size = "small",
    srcs = ["test/error_reporter_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "generate_lsp_messages_test",
    size = "small",
    srcs = ["test/generate_lsp_messages_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "lsp_file_updates_test",
    size = "small",
    srcs = ["test/lsp_file_updates_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "lsp_preprocessor_test",
    size = "small",
    srcs = ["test/lsp_preprocessor_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_test(
    name = "lsp_test",
    size = "small",
    srcs = ["test/lsp_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

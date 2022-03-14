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
        "lsp_messages_gen.h",
        "lsp_messages_gen.cc",
        "lsp_messages_enums_gen.h",
        "lsp_messages_enums_gen.cc",
        "LSPFileUpdates.h",
        "DefLocSaver.h",
        "watchman/WatchmanProcess.h",
        "AbstractRenamer.h",
        "NextMethodFinder.h",
        "LocalVarFinder.h",
        "FieldFinder.h",
        "json_enums.h",
        "LocalVarSaver.h",
        "ShowOperation.h",
        "lsp_messages_gen_helpers.h",
        "LSPIndexer.h",
        "LSPTask.h",
        "LSPTypechecker.h",
        "LSPTypecheckerCoordinator.h",
        "UndoState.h",
        "ErrorReporter.h",
        "ErrorFlusherLSP.h",
        "QueryCollector.h",
    ],
    hdrs = [
        "LSPConfiguration.h",
        "LSPIndexedFileStore.h",
        "LSPInput.h",
        "LSPMessage.h",
        "LSPOutput.h",
        "LSPPreprocessor.h",
        "LSPQuery.h",
        "MoveMethod.h",
        "json_types.h",
        "lsp.h",
        "wrapper.h",
    ],
    # Required for compiling //experimental/rubyfmt on Mac with dynamic linking
    # webasm has no options specified since it links against a no-op rubyfmt
    linkopts = select({
        "//conditions:default": [],
        "//experimental/rubyfmt:enable_rubyfmt": [
            "-lcrypt",
            "-lrt",
            "-lz",
            "-lgmp",
        ],
        "//experimental/rubyfmt:enable_rubyfmt_darwin": [
            "-framework Foundation",
        ],
    }),
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
        "//experimental/rubyfmt",
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

genrule(
    name = "generate_lsp_messages_h",
    outs = [
        "lsp_messages_gen.h",
        "lsp_messages_gen.cc",
        "lsp_messages_enums_gen.h",
        "lsp_messages_enums_gen.cc",
    ],
    cmd = "$(location :generate_lsp_messages) $(location lsp_messages_gen.h) $(location lsp_messages_gen.cc) \
             $(location lsp_messages_enums_gen.h) $(location lsp_messages_enums_gen.cc) && \
             $(location //tools:clang-format) -i $(location lsp_messages_gen.h) $(location lsp_messages_gen.cc) \
             $(location lsp_messages_enums_gen.h) $(location lsp_messages_enums_gen.cc)",
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
        "@doctest",
        "@doctest//:doctest_main",
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
        "@doctest",
        "@doctest//:doctest_main",
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
        "@doctest",
        "@doctest//:doctest_main",
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
        "@doctest",
        "@doctest//:doctest_main",
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
        "@doctest",
        "@doctest//:doctest_main",
    ],
)

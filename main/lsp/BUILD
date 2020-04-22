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
        "NextMethodFinder.h",
        "LocalVarFinder.h",
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
    ],
    hdrs = [
        "LSPConfiguration.h",
        "LSPInput.h",
        "LSPMessage.h",
        "LSPOutput.h",
        "LSPPreprocessor.h",
        "json_types.h",
        "lsp.h",
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
        "//main/cache",
        "//main/options",
        "//main/pipeline",
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
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "generate_lsp_messages_test",
    size = "small",
    srcs = ["test/generate_lsp_messages_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "lsp_file_updates_test",
    size = "small",
    srcs = ["test/lsp_file_updates_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "lsp_preprocessor_test",
    size = "small",
    srcs = ["test/lsp_preprocessor_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "lsp_test",
    size = "small",
    srcs = ["test/lsp_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
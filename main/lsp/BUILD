cc_library(
    name = "lsp",
    srcs = glob([
        "*.cc",
        "requests/*.cc",
        "watchman/*.cc",
    ]) + ["lsp_messages_gen.cc"],
    hdrs = [
        "DefLocSaver.h",
        "LSPMessage.h",
        "LocalVarSaver.h",
        "json_types.h",
        "lsp.h",
        "lsp_messages_gen.h",
        "lsp_messages_gen_helpers.h",
        "watchman/WatchmanProcess.h",
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
    ],
    cmd = "$(location :generate_lsp_messages) $(location lsp_messages_gen.h) $(location lsp_messages_gen.cc) && \
             $(location //tools:clang-format) -i $(location lsp_messages_gen.h) $(location lsp_messages_gen.cc)",
    tools = [
        ":generate_lsp_messages",
        "//tools:clang-format",
    ],
)

cc_test(
    name = "lsp_test",
    size = "small",
    srcs = glob([
        "test/*.cc",
    ]),
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "lsp",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

load(":lsp_test.bzl", "lsp_test", "update_test")

sh_binary(
    name = "lsp_test_runner",
    srcs = ["lsp_test_runner.sh"],
    data = ["//main:sorbet"],
)

cc_test(
    name = "protocol_test_corpus",
    timeout = "long",
    srcs = [
        "ProtocolTest.cc",
        "ProtocolTest.h",
        "protocol_test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//core",
        "//main/lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "multithreaded_protocol_test_corpus",
    size = "medium",
    srcs = [
        "ProtocolTest.cc",
        "ProtocolTest.h",
        "multithreaded_protocol_test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//core",
        "//main/lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "watchman_test_corpus",
    size = "medium",
    srcs = [
        "ProtocolTest.cc",
        "ProtocolTest.h",
        "watchman_test_corpus.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        "//core",
        "//main/lsp",
        "//payload",
        "//test/helpers",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

test_suite(
    name = "lsp",
    tests = [lsp_test(script) for script in glob(["*/*.rec"])],
)

update_test()

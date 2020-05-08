cc_library(
    name = "doctest",
    hdrs = glob(["doctest/**/*.h"]),
    strip_include_prefix = "doctest",
    defines = ["DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "dummy-main",
    outs = ["dummy-main.cc"],
    cmd = """
    echo '#include "doctest/doctest.h"' > $@
    """,
)

cc_library(
    name = "doctest_main",
    srcs = glob(["doctest/**/*.h"]) + ["dummy-main.cc"],
    testonly = True,
    local_defines = ["DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "doctest_custom_main",
    srcs = glob(["doctest/**/*.h"]) + ["dummy-main.cc"],
    testonly = True,
    local_defines = ["DOCTEST_CONFIG_IMPLEMENT"],
    visibility = ["//visibility:public"],
)

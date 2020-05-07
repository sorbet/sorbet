cc_library(
    name = "doctest",
    hdrs = glob(["doctest/**/*.h"]),
    strip_include_prefix = "doctest",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "doctest_main",
    testonly = True,
    defines = ["DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"],
    visibility = ["//visibility:public"],
)

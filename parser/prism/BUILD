# Prism's allocator hooks (`PRISM_XALLOCATOR`), see prism_xallocator.h. Depended on by @prism itself, so it must not
# depend on anything that depends on @prism (`//common` does not). `includes` makes `#include "prism_xallocator.h"`
# resolve from Prism's own sources.
cc_library(
    name = "xallocator",
    srcs = ["xallocator.cc"],
    hdrs = [
        "PrismArena.h",
        "prism_xallocator.h",
    ],
    includes = ["."],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = ["//common"],
)

cc_test(
    name = "xallocator_test",
    size = "small",
    srcs = ["test/xallocator_test.cc"],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//tools:__pkg__"],
    deps = [
        ":xallocator",
        "@doctest//doctest",
        "@doctest//doctest:main",
    ],
)

cc_library(
    name = "prism",
    srcs = [
        "Factory.cc",
        "Parser.cc",
    ],
    hdrs = [
        "Factory.h",
        "Helpers.h",
        "Parser.h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":xallocator",
        "//core",
        "//parser",  # Legacy parser, needed for translating to its nodes defined in `parser/Node_gen.h`
        "@prism",
    ],
)

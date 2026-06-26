cc_library(
    name = "sorbet_version",
    hdrs = [
        "sorbet_version.h",
        ":sorbet_version_defines_h",
    ],
    linkstatic = select({
        "//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [],  # this should have _no_ dependencies, we want it to be very light and not impose stuff
)

config_setting(
    name = "be-versioned",
    values = {
        "define": "versioned=true",
    },
)

_GENERATE_DEFINES = r"""BEGIN {
    print "#ifndef SORBET_VERSION_DEFINES_H"
    print "#define SORBET_VERSION_DEFINES_H"
    print "#define BUILD_RELEASE 1"
}
/STABLE_BUILD_SCM_REVISION/ { printf "#define STABLE_BUILD_SCM_REVISION \"%s\"\n", $$2 }
/STABLE_BUILD_SCM_COMMIT_COUNT/ { printf "#define STABLE_BUILD_SCM_COMMIT_COUNT %s\n", $$2 }
/STABLE_BUILD_SCM_CLEAN/ { printf "#define STABLE_BUILD_SCM_CLEAN \"%s\"\n", $$2 }
/BUILD_TIMESTAMP/ { printf "#define BUILD_TIMESTAMP %s\n", $$2 }
END {
    print "#endif"
}
"""

genrule(
    name = "sorbet_version_defines_h",
    outs = ["sorbet_version_defines.h"],
    cmd = select({
        "be-versioned": "awk '{}' bazel-out/stable-status.txt bazel-out/volatile-status.txt > $@".format(_GENERATE_DEFINES),
        "//conditions:default": "touch $@",
    }),
    stamp = select({
        "be-versioned": 1,
        "//conditions:default": 0,
    }),
    tags = ["manual"],
)

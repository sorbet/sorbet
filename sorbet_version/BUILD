cc_library(
    name = "sorbet_version",
    srcs = select({
        "be-versioned": ["sorbet_version_gen.cc"],
        "//conditions:default": ["sorbet_version.cc"],
    }),
    hdrs = ["sorbet_version.h"],
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

genrule(
    name = "sorbet_version_gen_cc",
    srcs = ["sorbet_version.cc"],
    outs = ["sorbet_version_gen.cc"],
    cmd = """
    scm_status=$$(grep STABLE_BUILD_SCM_CLEAN bazel-out/stable-status.txt | sed 's/^STABLE_BUILD_SCM_CLEAN //')
    scm_revision=$$(grep STABLE_BUILD_SCM_REVISION bazel-out/stable-status.txt | sed 's/^STABLE_BUILD_SCM_REVISION //')
    scm_commit_count=$$(grep STABLE_BUILD_SCM_COMMIT_COUNT bazel-out/stable-status.txt | sed 's/^STABLE_BUILD_SCM_COMMIT_COUNT //')
    build_timestamp=$$(grep BUILD_TIMESTAMP bazel-out/volatile-status.txt | sed 's/^BUILD_TIMESTAMP //')
    cat "$<" \\
      | sed 's/BUILD_RELEASE/true/g' \\
      | sed 's/STABLE_BUILD_SCM_CLEAN/"'"$${scm_status}"'"/g' \\
      | sed 's/STABLE_BUILD_SCM_REVISION/"'"$${scm_revision}"'"/g' \\
      | sed 's/STABLE_BUILD_SCM_COMMIT_COUNT/"'"$${scm_commit_count}"'"/g' \\
      | sed 's/BUILD_TIMESTAMP/'"$${build_timestamp}"'/g' \\
      > "$@"
    """,
    stamp = 1,
    tags = ["manual"],
)

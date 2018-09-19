cc_library(
    name = "version",
    srcs = select({
        "be-versioned": ["version_gen.cc"],
        "//conditions:default": ["version.cc"],
    }),
    hdrs = ["version.h"],
    visibility = ["//visibility:public"],
)

config_setting(
    name = "be-versioned",
    values = {
        "define": "release=true",
    },
)

genrule(
    name = "version_gen_cc",
    srcs = ["version.cc"],
    outs = ["version_gen.cc"],
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
)

filegroup(
    name = "runtime_deps",
    srcs = [
        "{{site_bin}}/bundle",
        "{{site_bin}}/bundler",
    ] + glob([{{site_ruby_glob}}]),
    visibility = ["//visibility:public"],
)

sh_binary(
    name = "bundler",
    srcs = ["bundle.sh"],
    data = [":runtime_deps"],
    visibility = ["//visibility:public"],
    deps = ["@bazel_tools//tools/bash/runfiles"],
)

sh_binary(
    name = "bundle",
    srcs = ["bundle.sh"],
    data = [":runtime_deps"],
    visibility = ["//visibility:public"],
    deps = ["@bazel_tools//tools/bash/runfiles"],
)

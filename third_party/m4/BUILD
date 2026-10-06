load("@rules_m4//m4/internal:toolchain.bzl", "m4_toolchain_info")
load(":toolchain.bzl", "m4_launcher")

m4_launcher(
    name = "m4",
    install = "@m4_src//:install",
    visibility = ["//visibility:public"],
)

# NOTE: `m4_toolchain_info` is technically an internal API from rules_m4.
# It seems to be publicly accessible anyways, but if we upgrade rules_m4 at
# some point, it's possible it becomes inaccessible.
m4_toolchain_info(
    name = "toolchain_info",
    m4_tool = ":m4",
)

toolchain(
    name = "toolchain",
    toolchain = ":toolchain_info",
    toolchain_type = "@rules_m4//m4:toolchain_type",
    visibility = ["//visibility:public"],
)

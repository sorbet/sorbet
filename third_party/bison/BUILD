load("@rules_bison//bison/internal:toolchain.bzl", "bison_toolchain_info")
load(":toolchain.bzl", "bison_launcher")

bison_launcher(
    name = "bison",
    install = "@bison_src//:install",
)

# NOTE: `bison_toolchain_info` is technically an internal API from rules_bison.
# It seems to be publicly accessible anyways, but if we upgrade rules_bison at
# some point, it's possible it becomes inaccessible.
bison_toolchain_info(
    name = "toolchain_info",
    bison_tool = ":bison",
)

toolchain(
    name = "toolchain",
    toolchain = ":toolchain_info",
    toolchain_type = "@rules_bison//bison:toolchain_type",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "payload",
    srcs = [
        "Payload.cc",
        "Payload_gen.cc",
        "payload.h",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "@org_llvm_darwin",
    ],
)

load("@com_stripe_ruby_typer//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one

clang_tool("llvm-as")

cc_binary(
    name = "generate_load_payload",
    srcs = [
        "tools/generate_load_payload.cc",
    ],
    visibility = ["//tools:__pkg__"],
    deps = [
        "@com_stripe_ruby_typer//common",
    ],
)

genrule(
    name = "generate_payload_gen_cc",
    srcs = ["payload.bc"],
    outs = [
        "Payload_gen.cc",
    ],
    cmd = "$(location :generate_load_payload) $(location payload.bc) $(location Payload_gen.cc)",
    tools = [
        ":generate_load_payload",
    ],
)

genrule(
    name = "generate_payload_bc",
    srcs = ["payload.ll"],
    outs = [
        "payload.bc",
    ],
    cmd = "$(location :llvm-as) -o=$(location payload.bc) $(location payload.ll)",
    tools = [
        ":llvm-as",
    ],
)

# todo: we should generate the `ll` file with clang instead of having it commited to the repo
# this will additionally make us specify a specific version of ruby in build
# this will involve:
#  - running something equivalent to `ruby extconf.rb` to generate the makefile. Find the `extconf.rb` in `demo` folder.
#  - calling `make payload.ll` to generate the ll file for people's viewing
#clang_tool("clang++") will give the "right" clang.

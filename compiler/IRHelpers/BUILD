cc_library(
    name = "IRHelpers",
    srcs = [
        "IRHelpers.cc",
        "IRHelpers.h",
        "IRHelpers_gen.cc",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "@org_llvm_darwin",
    ],
)

load("@com_stripe_ruby_typer//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one

clang_tool("llvm-as")
clang_tool("llvm-dis")
clang_tool("llvm-ar")

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

cc_library(
    name = "pseudo_lib",
    copts = ["-Wno-gcc-compat", "-flto=thin", "-emit-llvm", "-std=c99"],
    linkstatic = 1, # we are doing this only for llvm ir
    srcs = ["payload.c"],
    deps = ["@ruby_2_4_3//:ruby_private_headers", "@ruby_2_4_3//:ruby_headers"]
)

sh_binary(
    name = "payload_postprocess",
    srcs = ["tools/postprocess_payload.sh"],
)

genrule(
    name = "generate_payload_bc_orig",
    outs = [
        "payload.bc.orig",
    ],
    cmd = "$(location :llvm-ar) -x $(location :pseudo_lib) payload.o && mv payload.o $(location payload.bc.orig)" ,
    tools = [
        ":llvm-ar", ":pseudo_lib", 
    ],
)

genrule(
    name = "generate_payload_ll",
    srcs = [
        "payload.bc.orig",
    ],
    outs = [
        "payload.ll",
    ],
    cmd = "$(location :llvm-dis) -o $(location payload.ll) $(location :payload.bc.orig) && $(location :payload_postprocess) $(location payload.ll)",
    tools = [
        ":llvm-dis", ":payload_postprocess",
    ],
)


genrule(
    name = "generate_payload_gen_cc",
    srcs = ["payload.bc"],
    outs = [
        "IRHelpers_gen.cc",
    ],
    cmd = "$(location :generate_load_payload) $(location payload.bc) $(location IRHelpers_gen.cc)",
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

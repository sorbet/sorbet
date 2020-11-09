cc_library(
    name = "Payload",
    srcs = glob([
        "*.cc",
        "*.h",
    ]) + [
        "Payload_gen.cc",
    ],
    linkstatic = select({
        "@com_stripe_ruby_typer//tools/config:linkshared": 0,
        "//conditions:default": 1,
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//compiler/Core",
    ],
)

load("@com_stripe_ruby_typer//tools:clang.bzl", "clang_tool")  # todo: this should be decoupled and use the library toolchain, not the compiler one

clang_tool("llvm-as")

clang_tool("llvm-dis")

clang_tool("llvm-ar")

clang_tool("llvm-link")

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

filegroup(
    name = "vm_payload_srcs",
    srcs = [
        "PayloadIntrinsics.c",
        "payload.c",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "pseudo_lib",
    srcs = [
        "PayloadIntrinsics.c",
        "code-payload.c",
    ],
    copts = [
        "-Wno-gcc-compat",
        "-fembed-bitcode",
        "-std=c99",
        "-O1",
        "-g",
        "-fPIC",
        "-march=x86-64",
    ],
    # disable opt flags, as they add `-ffunction-sections` and `-fdata-sections`
    # See https://github.com/bazelbuild/bazel/issues/8706 for why `nocopts` is gone
    features = ["-opt"],
    linkstatic = 1,  # we are doing this only for llvm ir
    visibility = ["//visibility:public"],
    deps = [
        "@com_stripe_ruby_typer//sorbet_version",
        "@sorbet_ruby//:headers",
        "@sorbet_ruby//:headers-internal",
    ],
)

sh_binary(
    name = "payload_postprocess",
    srcs = ["tools/postprocess_payload.sh"],
)

sh_binary(
    name = "generate_payload_bc_orig_generator",
    srcs = ["tools/generate_payload_bc_orig_generator.sh"],
)

genrule(
    name = "generate_payload_bc_orig",
    outs = [
        "payload.bc.orig",
    ],
    cmd = "$(location :generate_payload_bc_orig_generator) $(location :llvm-ar) $(location :llvm-link) $(location payload.bc.orig) $(location //third_party/xxd) $(locations :pseudo_lib)",
    tools = [
        ":generate_payload_bc_orig_generator",
        ":llvm-ar",
        ":llvm-link",
        ":pseudo_lib",
        "//third_party/xxd",
    ],
    visibility = ["//visibility:public"],
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
        ":llvm-dis",
        ":payload_postprocess",
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

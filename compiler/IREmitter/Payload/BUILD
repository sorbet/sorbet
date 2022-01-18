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
        "serialize-runtime.c",
        "sorbet-t-modules.c",
        "vm-payload.c",
    ],
    visibility = ["//visibility:public"],
)

payload_files = [
    (
        "vm_payload_for_compile_commands",
        ["vm-payload.c"],
    ),
    (
        "pseudo_lib",
        [
            "PayloadIntrinsics.c",
            "codegen-payload.c",
        ],
    ),
]

[cc_library(
    name = name,
    srcs = srcs,
    copts = [
        "-Wno-gcc-compat",
        "-fembed-bitcode",
        # Note that we use -std=gnu99 here (instead of -std=c99) because we
        # borrow a lot of code from Ruby (via, e.g., internal.h), and that code
        # sometimes depends on GNU extensions (e.g. the "typeof" keyword).
        "-std=gnu99",
        "-O1",
        "-g",
        "-fPIC",
        "-march=x86-64",

        # The definition of id_intern in ruby.h triggers this warning, and it's
        # pretty noisy.
        "-Wno-compound-token-split-by-macro",
    ],
    # disable opt flags, as they add `-ffunction-sections` and `-fdata-sections`
    # See https://github.com/bazelbuild/bazel/issues/8706 for why `nocopts` is gone
    features = ["-opt"],
    linkstatic = 1,  # we are doing this only for llvm ir
    visibility = ["//visibility:public"],
    deps = [
        "@com_stripe_ruby_typer//sorbet_version",
        "@sorbet_ruby_2_7_for_compiler//:headers",
        "@sorbet_ruby_2_7_for_compiler//:headers-internal",
    ],
) for name, srcs in payload_files]

cc_binary(
    name = "postprocess_payload",
    srcs = ["tools/postprocess_payload.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "@llvm//:BitWriter",
        "@llvm//:Core",
        "@llvm//:IRReader",
        "@llvm//:Support",
    ],
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
    name = "generate_payload_bc",
    srcs = [
        "payload.bc.orig",
    ],
    outs = [
        "payload.bc",
    ],
    cmd = "$(location :postprocess_payload) $(location payload.bc.orig) $(location payload.bc)",
    tools = [
        ":postprocess_payload",
    ],
)

genrule(
    name = "generate_payload_ll",
    srcs = [
        "payload.bc",
    ],
    outs = [
        "payload.ll",
    ],
    cmd = "$(location :llvm-dis) $(location payload.bc) -o $(location payload.ll)",
    tools = [
        ":llvm-dis",
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

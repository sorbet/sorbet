cc_binary(
        name="main",
        deps=["//experimental/third_party/cargo/librubyfmt", "@ruby_2_6//:rubyfmt-static-deps", "//experimental/third_party/cargo/librubyfmt:rubyfmt_macro_wrappers"],
        srcs=["main.c", "rubyfmt.h"],
        linkopts = select({
            "@com_stripe_ruby_typer//tools/config:darwin": ["-framework Foundation"],
            "//conditions:default": [""],
        }),
        )

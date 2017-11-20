cc_library(
    name = "ast",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = [
            "*_test.cc",

            # workaround https://github.com/flycheck/flycheck/issues/248 in emacs
            "flycheck_*",
        ],
    ),
    hdrs = [
        "ast.h",
    ],
    copts = ["--std=c++14"],
    visibility = ["//visibility:public"],
    deps = [
        "//common",
        "//core",
    ],
)

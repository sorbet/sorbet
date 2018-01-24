genrule(
    name = "jemalloc_genrule",
    srcs = glob([
        "**/*.cc", "**/*.c", "**/*.cpp",
        "**/*.h" ,
        "**/*.in", "**/*.sh",
        "**/*.ac", "**/*.m4" , "**/*.guess",  "**/*.sub", "**/install-sh"]
    ),
    outs = ["lib/libjemalloc.a", "include/jemalloc/jemalloc.h"],
    # workaround https://github.com/bazelbuild/bazel/issues/3128
    cmd = "pushd $$(dirname $(location autogen.sh))  && \
           ./autogen.sh  && \
            make build_lib_static -j4  && \
            popd &&\
            mv $$(dirname $(location autogen.sh))/lib/libjemalloc.a $(location lib/libjemalloc.a) && \
            mv $$(dirname $(location autogen.sh))/include/jemalloc/jemalloc.h $(location include/jemalloc/jemalloc.h)",
)

cc_library(
    name = "jemalloc",
    srcs = ["lib/libjemalloc.a"],
    hdrs = ["include/jemalloc/jemalloc.h"],
    visibility = ["//visibility:public"],
    linkopts = select({
                       "@com_stripe_ruby_typer//tools/config:linux": ["-ldl"], # side step https://github.com/jemalloc/jemalloc/issues/948
                       "//conditions:default": [],
                   }),
)

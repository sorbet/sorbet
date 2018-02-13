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
    cmd = select({
                           "@com_stripe_ruby_typer//tools/config:linux":
                                      "export WORKSPACE_TOP=$$(pwd)/ && \
                                       export CC=$$(realpath $(CC)) && \
                                       export CXX=$$(realpath $(CC)) && \
                                       export AR=$$(realpath $(AR)) && \
                                       export NM=$$(realpath $(NM)) && \
                                       export OBJCOPY=$$(realpath $(OBJCOPY)) && \
                                       export CFLAGS=$(CC_FLAGS) && \
                                       export CXXFLAGS=$(CC_FLAGS) && \
                                       pushd $$(dirname $(location autogen.sh))  && \
                                       EXTRA_CFLAGS=-flto=thin EXTRA_CXXFLAGS=-flto=thin LDFLAGS='-flto=thin -fuse-ld=lld' ./autogen.sh --without-export && \
                                       make build_lib_static -j4  && \
                                       popd &&\
                                       mv $$(dirname $(location autogen.sh))/lib/libjemalloc.a $(location lib/libjemalloc.a) && \
                                       mv $$(dirname $(location autogen.sh))/include/jemalloc/jemalloc.h $(location include/jemalloc/jemalloc.h)",
                           "@com_stripe_ruby_typer//tools/config:darwin": # os x bazel has wrapper scripts around CC, and makes does not link shebangs
                                      "export WORKSPACE_TOP=$$(pwd)/ && \
                                       export CC=\"sh $$(realpath $(CC))\" && \
                                       export CXX=\"sh $$(realpath $(CC))\" && \
                                       export NM=$$(realpath $(NM)) && \
                                       export OBJCOPY=$$(realpath $(OBJCOPY)) && \
                                       export MY_LOCATION=$$(pwd)/external/local_config_cc/ && \
                                       export CFLAGS=$(CC_FLAGS) && \
                                       export CXXFLAGS=$(CC_FLAGS) && \
                                       pushd $$(dirname $(location autogen.sh))  && \
                                       EXTRA_CFLAGS=-flto=thin EXTRA_CXXFLAGS=-flto=thin LDFLAGS=-flto=thin ./autogen.sh --without-export  && \
                                       make build_lib_static -j4  && \
                                       popd &&\
                                       mv $$(dirname $(location autogen.sh))/lib/libjemalloc.a $(location lib/libjemalloc.a) && \
                                       mv $$(dirname $(location autogen.sh))/include/jemalloc/jemalloc.h $(location include/jemalloc/jemalloc.h)",
                       }),
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

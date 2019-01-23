# There's a bunch of workarounds in this build command.
# - We workaround https://github.com/bazelbuild/bazel/issues/3128
# - We need to convert CC et al to absolute paths, but we can't use
#   `realpath` because we need to run them out of a workspace-relative
#   path for our CROSSTOOL to find clang.
# - CC et al are shell-script wrappers on macOS, and we need to add an
#   explicit `sh ...` for reasons we don't fully understand.
# - We have to set AR on Linux, but on macOS that points to `libtool`
#   which has a different calling convention, so we leave it unset
#   there and let configure find the default `ar`
JEMALLOC_BUILD_COMMAND = """
  absolutize() {
   local path=$$1
   if [ "$${path#/}" = "$$path" ]; then
     path="$$(pwd)/$$path"
   fi
   if [ "$${path%.sh}" != "$$path" ]; then
     path="sh $$path"
   fi
   echo "$$path"
  }
  export CC=$$(absolutize $(CC))
  export CXX=$$(absolutize $(CC))
  [ "$$(uname)" = "Linux" ] && export AR=$$(absolutize $(AR))
  export NM=$$(absolutize $(NM))
  export OBJCOPY=$$(absolutize $(OBJCOPY))
  export CFLAGS=$(CC_FLAGS)
  export CXXFLAGS=$(CC_FLAGS)
  pushd $$(dirname $(location autogen.sh)) && \
  EXTRA_CFLAGS=-flto=thin EXTRA_CXXFLAGS='-flto=thin -stdlib=libc++' LDFLAGS="-flto=thin$$([ "$$(uname)" = "Linux" ] && echo " -fuse-ld=lld")" ./autogen.sh --without-export && \
  make build_lib_static -j4  && \
  popd && \
  mv $$(dirname $(location autogen.sh))/lib/libjemalloc.a $(location lib/libjemalloc.a) && \
  mv $$(dirname $(location autogen.sh))/include/jemalloc/jemalloc.h $(location include/jemalloc/jemalloc.h)
"""

genrule(
    name = "jemalloc_genrule",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.c",
            "**/*.cpp",
            "**/*.h",
            "**/*.in",
            "**/*.sh",
            "**/*.ac",
            "**/*.m4",
            "**/*.guess",
            "**/*.sub",
            "**/install-sh",
        ],
    ),
    outs = [
        "lib/libjemalloc.a",
        "include/jemalloc/jemalloc.h",
    ],
    cmd = JEMALLOC_BUILD_COMMAND,
    toolchains = ["@bazel_tools//tools/cpp:current_cc_toolchain"],
)

cc_library(
    name = "jemalloc",
    srcs = [":jemalloc_genrule"],
    hdrs = ["include/jemalloc/jemalloc.h"],
    linkopts = select({
        "@com_stripe_ruby_typer//tools/config:linux": ["-ldl"],  # side step https://github.com/jemalloc/jemalloc/issues/948
        "//conditions:default": [],
    }),
    linkstatic = 1,
    visibility = ["//visibility:public"],
)

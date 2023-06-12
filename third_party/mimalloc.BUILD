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
MIMALLOC_BUILD_COMMAND = """
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
  export PATH="/usr/local/bin:$$PATH" # find autoconf on mac
  export PATH="/opt/homebrew/bin:$$PATH" # find autoconf on mac arm64
  export PATH="/home/linuxbrew/.linuxbrew/bin:$$PATH" # find autoconf on linux using Linuxbrew
  export CC=$$(absolutize $(CC))
  export CXX=$$(absolutize $(CC))
  [ "$$(uname)" = "Linux" ] && export AR=$$(absolutize $(AR))
  export NM=$$(absolutize $(NM))
  export OBJCOPY=$$(absolutize $(OBJCOPY))
  export CFLAGS=$(CC_FLAGS)
  export CXXFLAGS=$(CC_FLAGS)
  export LTOFLAGS="$$([ "$$(uname)" = "Linux" ] && echo "-flto=thin -Wl,--thinlto-jobs=all")" # todo: on next clang toolchain upgrade, check if it's fixed and we can re-enable thinlto on mac
  export EXTRA_CFLAGS="$${LTOFLAGS}"
  export EXTRA_CXXFLAGS="-stdlib=libc++ $${EXTRA_CFLAGS}"

  LDFLAGS="$${LTOFLAGS}"

  AUTOGEN_FLAGS=
  case "$$(uname)" in
    Linux)
      LDFLAGS="$${LDFLAGS} -fuse-ld=lld"
      ;;
    Darwin)
      LDFLAGS="$${LDFLAGS} -mlinker-version=400"
      AUTOGEN_FLAGS=--with-lg-vaddr=48
      ;;
  esac

  export LDFLAGS

  # we need to include CFLAGS in CPPFLAGS so that the --sysroot flag makes it in
  export CPPFLAGS="$${CFLAGS}"

  mkdir -p $$(dirname $(location CMakeLists.txt))/out/release
  pushd $$(dirname $(location CMakeLists.txt))/out/release > /dev/null

  if compile_output=$$(cmake ../.. && make); then
    popd > /dev/null
    mv $$(dirname $(location CMakeLists.txt))/out/release/libmimalloc.a $(location lib/libmimalloc.a)
  else
    printf '%s\n' "$${compile_output}"
    exit 1
  fi

"""

genrule(
    name = "mimalloc_genrule",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.c",
            "**/*.cpp",
            "**/*.h",
            "**/*.in",
            "**/*.sh",
            "**/*.cmake",
            "**/CMakeLists.txt",
        ],
    ),
    outs = [
        "lib/libmimalloc.a",
    ],
    cmd = MIMALLOC_BUILD_COMMAND,
    toolchains = [
        "@bazel_tools//tools/cpp:cc_flags",
        "@bazel_tools//tools/cpp:current_cc_toolchain",
    ],
)

cc_library(
    name = "mimalloc",
    srcs = [":mimalloc_genrule"],
    hdrs = ["include/mimalloc.h"],
    linkopts = select({
        "@platforms//os:linux": ["-ldl"],  # side step https://github.com/jemalloc/jemalloc/issues/948
        "//conditions:default": [],
    }),
    linkstatic = 1,
    visibility = ["//visibility:public"],
)

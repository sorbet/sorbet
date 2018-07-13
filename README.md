# `Sorbet` -- an experimental Ruby typechecker

This repository contains `Sorbet`, a work-in-progress experiment
aimed at producing a static typechecker for a subset of Ruby.

You can read more in our [design doc](https://hackpad.corp.stripe.com/Design-Doc-sorbet-zd1LGHPfpvW).

It is still in its early days and should be considered alpha-quality.
You are welcome to try it, though, but your experience might still be rough.

# Running
Install [Dependencies](#dependencies) first.

You'll need to build Sorbet.  In order to build the production version run:

```
./bazel build //main:sorbet -c opt
```

The resulting executable will be runable with:

```
bazel-bin/main/sorbet my_file1.rb my_file2.rb
```

It should be statically linked and have no dependencies, so feel free to copy it.

In order to build a release version please run `./bazel build //main:sorbet --config=release`.

# Developing on `Sorbet`

We build using [bazel](https://bazel.build/); Run: 

```
./bazel test //... --config=dbg
```

to build and run the tests(`//...` stands for "everything", you literaly need to type `//...`).

During development we pass `--config=dbg` to build debug version of `sorbet` that also includes additional sanity checks that check consistency of internal datastructures (see `sanityCheck` methods in code). This build can also print pretty stack traces on failures when run on Mac (adding Linux support is welcome)

Our CI runs builds under `--config=ci`. Those are optimized builds that include sanity checks and additionally include checks added by [address sanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer) and [undefined behaviour sanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).

This project can take some time to build, in particular, if you change build configurations frequently. We have found that defining following bazel caching options (in `sorbet/.bazelrc`) helps. Note that those options could sometimes unveil bazel bugs, so if something is weird, disable them and nuke the cache by deleting files in it.

```
# mkdir /tmp/bazel-cache
build  --experimental_local_disk_cache --experimental_local_disk_cache_path=/tmp/bazel-cache
test  --experimental_local_disk_cache --experimental_local_disk_cache_path=/tmp/bazel-cache
```

# Code conventions

 - use smart pointers for storage, references for arguments;
 - no c-style allocators. Use vectors instead.
 - undefined behaviour is prohibited. All builds have [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) enabled;
 - all memory accesses are checked by [ASan](https://github.com/google/sanitizers/wiki/AddressSanitizer).

## Dependencies

- [ragel](http://www.colm.net/open-source/ragel/);
- [bison](https://www.gnu.org/software/bison/);
- [ruby](https://www.ruby-lang.org/en/);
- [clang](https://clang.llvm.org/), but we download it as part of the build.
  [clang-format](https://clang.llvm.org/docs/ClangFormat.html), but we download it as part of hte build.
- [autoconf](https://www.gnu.org/software/autoconf/autoconf.html) for building jemalloc from source;
- [gnu coreutils](http://www.gnu.org/software/coreutils/coreutils.html) and [gnu parallel](https://www.gnu.org/software/parallel/) are used by some of bash scripts;
- platform headers for [ncurses5](https://www.gnu.org/software/ncurses/).

### On macOS

```
brew install ragel bison autoconf coreutils parallel
```

#### Known Issues
If you see a build error that looks like,
```
Executing genrule @parser//:bison_parser failed
```
then you might need to run:
```
brew link bison --force
```
to symlink the correct version into `/usr/local/bin/` so it appears on your `$PATH`.

### On Ubuntu 16.04

```
sudo apt install ragel bison libncurses5-dev autoconf
```

And then copy `bazelrc-ubuntu` to `.bazelrc` to configure bazel to use
the `clang-4.0` toolchain.

You do not need to install `bazel`; The `./bazel` tool will download
and build an appropriate version of bazel. If you have a global
`bazel` binary, it will automatically dispatch to this tool instead.

## Formatting

We require that all `BUILD` files are formatted by the
[buildifier](https://github.com/bazelbuild/buildtools/tree/master/buildifier)
tool, and all C/C++ source by `clang-format`.

Run `tools/scripts/format_build_files.sh` and
`tools/scripts/format_cxx.sh` respectively to format all these files
in-place.

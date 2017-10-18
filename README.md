# `ruby-typer` -- an experimental Ruby typechecker

This repository contains `ruby-typer`, a work-in-progress experiment
aimed at producing a static typechecker for a subset of Ruby.

You can read more in our [design doc](https://hackpad.corp.stripe.com/Design-Doc-ruby-typer-zd1LGHPfpvW)

# Developing on `ruby-typer`

We build using [bazel](https://bazel.build/); Run `./bazel test
//test/...` to build and run the tests.

# Code convenctions

 - use smart pointers for storage, references for arguments;
 - no c-style allocators. Use vectors instead.
 - undefined behaviour is prohibited. All builds have [UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) enabled;
 - all memory accesses are checked by [ASan](https://github.com/google/sanitizers/wiki/AddressSanitizer).

## Dependencies

- [ragel](http://www.colm.net/open-source/ragel/)
- [bison](https://www.gnu.org/software/bison/)
- [ruby](https://www.ruby-lang.org/en/)
- [clang](https://clang.llvm.org/) at least version 4.0.0, and
  [clang-format](https://clang.llvm.org/docs/ClangFormat.html) at
  least version 5.0.0.

### On macOS

```
brew install ragel bison
```

### On Ubuntu 16.04

```
sudo apt install ragel bison clang-4.0
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

# sorbet_llvm

This repository contains the Sorbet compiler. It uses Sorbet and LLVM to emit
shared objects that conform to Ruby's C extension API given Ruby source files.

It also contains Sorbet Ruby, a set of scripts that build Ruby in a specific way
so that it can run shared objects that the Sorbet Compiler emits.

This README contains documentation specifically for contributing to sorbet_llvm.
You might also want to:

- Read the [design doc](http://go/srbc/design) for the Sorbet compiler.

- Read the [Sorbet Ruby at Stripe](http://go/sorbet_ruby) documentation.

If you are at Stripe, you might also want to see <http://go/types/internals> for
docs about Stripe-specific development workflows and historical Stripe context.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
## Table of Contents

- [Quickstart](#quickstart)
- [Learning how the Sorbet compiler works](#learning-how-the-sorbet-compiler-works)
- [Running Sorbet Ruby and the Sorbet compiler](#running-sorbet-ruby-and-the-sorbet-compiler)
- [Running the tests](#running-the-tests)
- [Writing tests](#writing-tests)
  - [Comparing LLVM IR](#comparing-llvm-ir)
  - [Comparing stderr](#comparing-stderr)
- [Debugging](#debugging)
  - [`.lldbinit`](#lldbinit)
  - [Debugging the Ruby VM](#debugging-the-ruby-vm)
- [C++ in `sorbet_llvm`](#c-in-sorbet_llvm)
- [Editor and environment](#editor-and-environment)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Quickstart

For the best chance of success:

- Use Ubuntu 16.04 (18.04 *might* work, 20.04 almost definitely won't work)
- Run `sudo apt-get install libncurses-dev`

Then run:

```bash
./bazel test //test:test
```

This will take a long time, as it's downloading everything and building
everything (including LLVM) from scratch.

> sorbet_llvm is tested on a very narrow set of machines and rarely built from
> scratch. It's likely that the first time you try to build it on a new machine
> you will encounter problems, and need to either install certain system-wide
> dependencies, or switch operating systems.
>

## Learning how the Sorbet compiler works

TODO(jez) compiler internals


## Running Sorbet Ruby and the Sorbet compiler

There's a fair amount of set up before you can run Sorbet Ruby on a file or load
a compiled Ruby file:

- Using `sig` and `T` depends on `sorbet-runtime`
- Compiling one or more Ruby files requires a folder for the LLVM IR and `*.so`
  files.
- Running a compiled Ruby file requires passing that folder and also including
  some Ruby monkeypatches to `require` find the artifacts.

There are some short scripts that handle all this bookkeeping:

```bash
# Interpret a file with Sorbet Ruby
test/run_ruby.sh     [-d] test/testdata/compiler/hello.rb

# Compile a file with the Sorbet compiler
test/run_sorbet.sh   [-d] test/testdata/compiler/hello.rb

# Compile and run a file
test/run_compiled.sh [-d] test/testdata/compiler/hello.rb
```

The optional `-d` flag to these scripts will launch the important command for
that script in `lldb` with all the right args, so you can set breakpoints before
running it.


## Running the tests

To run all the tests:

```
bazel test //... --config=dbg
```

(The `//...` literally means "all targets".)

To run a subset of the tests curated for faster iteration and development speed,
run:

```
bazel test test --config=dbg
```

Note that in bazel terms, the first `test` is a subcommand but the second `test`
is a shortcut for writing `//test:test`, so we're being a bit cute here.

By default, all test output goes into files. To also print it to the screen:

```
bazel test //... --config=dbg --test_output=errors
```

If any test failed, you will see two pieces of information printed:

```
1.  //test:test_testdata/compiler/hello                                      FAILED in 0.1s
2.    /home/jez/.cache/bazel/.../test/test_testdata/compiler/hellow/test.log
```

1.  the test's target (in case you want to run just this test again with `bazel
    test <target>`)
2.  a (runnable) file containing the test's output

To see the failing output, either:

- Re-run `bazel test` with the `--test_output=errors` flag
- Copy/paste the `*.log` file and run it (the output will open in `less`)


## Writing tests

We write tests by adding files to subfolders of the `test/` directory.
Every compiler test works like this:

- Write a Ruby file
- Interpret the Ruby file with Sorbet Ruby
- Compile and run the compiled output
- Compare the resulting exit codes, stdouts, and stderrs (they must match to
  pass)

These Ruby file tests live in `test/testdata/`.

Thus, it's very important to **print things** in tests. If a test doesn't print
anything, then both the interpretted and compiled output will be empty and the
test will pass even if the compiler did something completely wrong.

### Comparing LLVM IR

A test can also have a `*.llo.exp` file. If this file is present, the test
runner will use the `llvm-diff` command line tool to assert that the optimized
LLVM IR output for a given `*.rb` file matches what's in the `*.llo.exp` file.
For example:

- [test/testdata/compiler/hello.rb](test/testdata/compiler/hello.rb)
- [test/testdata/compiler/hello.llo.exp](test/testdata/compiler/hello.llo.exp)

The `llvm-diff` command is not a textual diff. It is slightly aware of
functionally equivalent files (e.g., names of variables are different but
they're initialized and used the same way means they match).

The LLVM IR diff tests fail are **expected to fail** for most new changes. The
idea is that you should skim the diffs, make sure they look appropriate given
what was meant to change, and then record the new exp files with this command:

```bash
# Only reproducible if run on Linux, not macOS
tools/scripts/update_exp_files.sh
```

The expectations files are somewhat platform dependent, so they must be recorded
on a Linux machine.

After the expectation files have been edited on disk, use

```
git diff --color-words
```

for a better git diff that ignores irrelevant parts that have changed.

### Comparing stderr

Frequently, a test fails because the output on stderr is different. For example,
interpreted vs compiled backtraces usually are slightly different.

Rather than letting the test chunder an exception backtrace to stderr, consider
catching the expected exception and only printing the message.

If the stderr difference is not due to exceptions (or the behavior of the test
changes when introducing exception handling), add this special comment to the
top of the test:

```
# skip_stderr_check
```

With this comment, only the exit code and stdout of a test must match.

## Debugging

It is hard to debug something inside the Bazel test sandbox directly. Instead,
use the `test/run_*.sh` scripts (discussed above). These scripts have a `-d`
option that will drop into `lldb` before running.

### `.lldbinit`

We have a custom `.lldbinit` file in `sorbet_llvm` with some helper commands.
The new commands it makes available inside `lldb` are:

- `rubysourcemaps`
  - set up source maps Ruby VM source files (e.g., when looking at a `ruby`
    C-level backtrace)
- `llvmsourcemaps`
  - set up source maps for LLVM source files (e.g., when looking at exceptions
    thrown when compiling a Ruby file)
- `rubystack`
  - dump the current Ruby stack if stopped in a Ruby VM breakpoint

If you see a message about local `.lldbinit` files when running `lldb`, you can
either:

-   ignore all local .lldbinit files:

    ```
    # ~/.lldbinit
    settings set target.load-cwd-lldbinit false
    ```

-   run all local .lldbinit files:

    ```
    # ~/.lldbinit
    settings set target.load-cwd-lldbinit true
    ```

-   make the choice run-by-run:

    ```
    lldb (--local-lldbinit|--no-lldbinit) <normal lldb args...>
    ```

### Debugging the Ruby VM

Ruby code compiled by Sorbet is run by the Ruby VM. It helps to have an idea of
what functions are available in the Ruby VM to break on or call when stopped:

Suggested breakpoints:

- `rb_longjmp` - break on nearly any Ruby exception being raised
  - There are a handful of cases where breaking on `rb_longjmp` won't stop when
    a Ruby exception is raised, but I don't remember what they are.
- Find something in [insns.def](https://github.com/ruby/ruby/tree/master/insns.def)
  - All Ruby VM bytecode instructions' implementations are in `insns.def`
  - You can break on like `insns.def:762`

Suggested functions to call while stopped:

- `rb_backtrace()`
  - Print a Ruby-level backtrace right now
- `rb_id2name`
  - Convert an `ID` (Ruby interned string) to a C string
- `sorbet_dbg_p(...)`
  - Print the given Ruby `VALUE` like you had called `p ...` in Ruby

Other:

-   To debug Ruby running via Rbenv:

    ```
    lldb -- $(rbenv which ruby) [<args>...]
    ```

-   You can use the [`stop_in_debugger`] gem to set a C-level debugger
    breakpoint from Ruby source code.

[`stop_in_debugger`]: https://github.com/jez/stop_in_debugger


## C++ in `sorbet_llvm`

<!-- TODO(jez) This probably makes more sense in a potential sorbet_llvm internals doc -->

In Sorbet, any exception is a crash (Sorbet never raises exceptions for expected
behavior).

But this is not true in the Sorbet compiler. Certain user-expected outcomes are
powered by exceptions (see `AbortCompilation`). This means that an exception
might be thrown to interrupt you during normal execution and be recovered from
higher up. For example:

```cpp
unique_ptr<int> foo(unique_ptr<int> x) {
    auto *y = x.release();
    codeThatRaisesAbortCompilation();
    auto z = unique_ptr<int>(y);
    return z;
}
```

In this case, ownership of `x` is released, but an exception is thrown before
ownership of `y` is transfered to `z`. This means that the memory associated
with `y` will never be freed, but the `AbortCompilation` will be caught and
execution will continue, and this memory will have leaked.

It's best to just never use `unique_ptr::release` in sorbet_llvm.


## Editor and environment

Nearly all of the [Editor and
environment](https://github.com/sorbet/sorbet#editor-and-environment) tips from
the Sorbet repo apply here too.

Sorbet compiler-specific recommendations:

- Get yourself a syntax highlighting plugin for LLVM IR. For example:
  - <https://github.com/rhysd/vim-llvm>

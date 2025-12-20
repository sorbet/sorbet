<p align="center">
  <img alt="Sorbet Compiler logo" width="312" src="../docs/logo/sorbet-compiler-logo@2x.png">
</p>

# The Sorbet Compiler

This repository contains the Sorbet compiler. It uses Sorbet and LLVM to emit
shared objects that conform to Ruby's C extension API given Ruby source files.

It also contains Sorbet Ruby, a set of scripts that build Ruby in a specific way
so that it can run shared objects that the Sorbet Compiler emits.

This README contains documentation specifically for contributing to the compiler
portion of Sorbet.

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
- [C++ in the compiler](#c-in-the-compiler)
- [Editor and environment](#editor-and-environment)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Quickstart

For the best chance of success:

- Run `sudo apt-get install libncurses-dev libssl-dev`

Then run:

```bash
./bazel test //test:compiler
```

This will take a long time, as it's downloading everything and building
everything (including LLVM) from scratch.

> The Sorbet Compiler is tested on a very narrow set of machines and rarely
> built from scratch. It's likely that the first time you try to build it on a
> new machine you will encounter problems, and need to either install certain
> system-wide dependencies, or switch operating systems.


## Learning how the Sorbet Compiler works

TODO(jez) compiler internals


## Running Sorbet Ruby and the Sorbet Compiler

For a more complete walk through, see
[docs/running-compiled-code.md](/docs/running-compiled-code.md).

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
bazel test //test:compiler //test/cli/compiler --config=dbg
```

By default, all test output goes into files. To also print it to the screen:

```
bazel test //test:compiler //test/cli/compiler --config=dbg --test_output=errors
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
anything, then both the interpreted and compiled output will be empty and the
test will pass even if the compiler did something completely wrong.

### Verifying LLVM IR

We rely on LLVM's
[FileCheck](https://llvm.org/docs/CommandGuide/FileCheck.html) to verify
that the structure of the generated IR conforms to our expectations.  If you
are writing a test where you would like to verify the structure of the
generated code, you need to add a `# run_filecheck: <pass>` comment to the
file.  This comment will typically be added in the top header block of comments.

`<pass>` in the above comment can be one of three values:

- `INITIAL`, for verifying the LLVM IR immediately after it is generated by the Sorbet compiler;
- `LOWERED`, for verifying the LLVM IR after the Sorbet compiler's custom lowerings have been run;
- `OPT`, for verifying the LLVM IR after LLVM's suite of optimizations has been run.

`<pass>` corresponds to the value given to the `-check-prefix` option of FileCheck.

Please note that FileCheck will verify that your checks occur in the same
order in the LLVM IR file as they are written in the test file.  So, as an example,
if a file contains:

```
# OPT-LABEL: "func_Object#foo"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT: call i64 @rb_yield_values_kw
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT{LITERAL}: }
```

The following checks will be run in order on the optimized LLVM IR:

- `"func_Object#foo"` appears exactly once in the file;
- `call i64 @sorbet_getMethodBlockAsProc` does not appear between the above line and the next check;
- `call i64 @rb_yield_values_kw` appears;
- `call i64 @sorbet_getMethodBlockAsProc` does not appear between the previous line and the next check;
- a literal `}` appears on a line.

The first check and the last check are intended to limit the scope of the
contained checks to a single function definition.  The quotes in the first
check are intended to limit the check to the actual function definition and
avoid matching ancillary data associated with the function.  If we used
`INITIAL-LABEL:`, `INITIAL-NOT:`, `INITIAL:`, and so forth, the checks would
be run against the initial LLVM IR generated by the Sorbet compiler.

It is generally not a good idea to run checks for only a single pass:
running checks against two passes enables you to check whether the initial
structure was what you expected (and make things robust against
e.g. function renames) and whether the transformation you expected to take
place actually did take place.

### Comparing LLVM IR (deprecated)

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
# Can only be run on Linux.
tools/scripts/update_compiler_exp.sh
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

We have a custom `.lldbinit` file with some helper commands.
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


## C++ in the compiler

<!-- TODO(jez) This probably makes more sense in a potential compiler internals doc -->

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
ownership of `y` is transferred to `z`. This means that the memory associated
with `y` will never be freed, but the `AbortCompilation` will be caught and
execution will continue, and this memory will have leaked.

It's best to just never use `unique_ptr::release` in the compiler.


## Editor and environment

Nearly all of the [Editor and environment] tips from the main README apply here
too.

[Editor and environment]: https://github.com/sorbet/sorbet#editor-and-environment

Especially the `--disk_cache` suggestions. If you're working on a Stripe devbox,
you'll want to make the `.bazelrc.local` file on the devbox--it's gitignored by
default, so you'll want to make it directly on the devbox.

Sorbet compiler-specific recommendations:

- Get yourself a syntax highlighting plugin for LLVM IR. For example:
  - <https://github.com/rhysd/vim-llvm>

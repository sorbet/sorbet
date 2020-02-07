# Sorbet Compiler

This project uses the Control Flow Graph from Sorbet to emit some LLVM bytecode
which compiles to a Ruby C-extension which you should load after your Ruby code
to replaces some of your functions with more optimized versions.

# Getting Started

This project uses [Bazel](https://bazel.build/). Download all dependencies, build
the project and run the tests with a single command (warning this will take a while
as it has to build LLVM):

```
    ./bazel build //test:test
```

To just rebuild the compiler itself, run

```
    ./bazel test //main:sorbet
```

# Debugging

So you're getting a segfault when running ruby? Here are some helpful ways to
debug. First compile a debug version of ruby:

```
    bazel build @sorbet_ruby//:ruby --config=dbg
```

Now you can start it in `lldb` like so:

```
    lldb ./bazel-out/darwin-dbg/bin/external/sorbet_ruby/toolchain/bin/ruby -- <your_args_here>
```

Now when you run it there are some useful functions for debugging:

* `rb_id2name` - Print the underlaying data from an `ID`
* `*(RString*)str` - Print the string contents of a `VALUE` which already is a String
* `*(RString*)rb_class_name(klass)` - Print the name of a `VALUE` which is a Class

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

To get source maps while debugging, make sure that our `.lldbinit` was run, and
then use this custom LLDB command:

```
(lldb) rubysourcemaps
```


# Building on linux

`apt get install libncurses-dev`

# Differences from Sorbet development.
 - All code in sorbet_llvm should be exception safe.
   - the most common way to make some code non-exception safe is to use pointer juggling with `unique_ptr.release`. Don't use it in sorbet_llvm

# Reading exp file diffs
  With many changes in exp files being optimization changes, running `git diff --word-diff` gives better way to show the diff.

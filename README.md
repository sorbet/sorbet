# Sorbet Compiler

This project uses the Control Flow Graph from Sorbet to emit some LLVM bytecode
which compiles to a Ruby C-extension which you should load after your Ruby code
to replaces some of your functions with more optimized versions.

# Debugging

So you're getting a segfault when running ruby? Here are some helpful ways to
debug. First compile a debug version of ruby:

```
    bazel build @ruby_2_4_3//:ruby --config=dbg
```

Now you can start it in `lldb` like so:

```
    lldb ./bazel-out/darwin-dbg/bin/external/ruby_2_4_3/bin/ruby -- <your_args_here>
```

Now when you run it there are some useful functions for debugging:

* `rb_id2name` - Print the underlaying data from an `ID`

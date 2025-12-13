# Running compiled code

The instructions will help you compile and run ruby using the Sorbet Compiler on
Linux.

## Building `sorbet_ruby`

The Sorbet Compiler relies on additional runtime support that we provide by
linking it into the Ruby VM. As a result, it is not possible to run Sorbet
Compiler-generated artifacts in a stock Ruby VM. To build our custom VM, run the
following command:

```shell
$ bazel build @sorbet_ruby_2_7_for_compiler//:ruby.tar.gz \
    --config=release-linux \
    --crosstool_top=@bazel_tools//tools/cpp:toolchain
```

The flags that we give are:

- `--config=release-linux` - Build a release build.

  The resulting Ruby VM will include runtime assertions that prevent loading
  compiled artifacts emitted by a different revision of Sorbet (based on Git
  SHA).

- `--crosstool_top=@bazel_tools//tools/cpp:toolchain` - Build using the native
  C++ toolchain instead of the one that we manage in the Bazel sandbox. Without
  this flag the Ruby build will be configured to use the C++ compiler that lives
  in the bazel sandbox to build extensions with native code components, which
  doesn't relocate well.

Once this build has finished, the archive
`bazel-bin/external/sorbet_ruby_2_7_for_compiler/ruby.tar.gz` will contain the
build of Ruby 2.7 with our patches applied. Copy that TAR file out of the Bazel
tree and extract it in a scratchpad directory:

```shell
$ mkdir scratchpad
$ cp bazel-bin/external/sorbet_ruby_2_7_for_compiler/ruby.tar.gz scratchpad
$ tar -xf scratchpad/ruby.tar.gz -C scratchpad
```

You can verify that it's doing the right thing by running:

```shell
$ scratchpad/bin/ruby --version
ruby 2.7.2p137 (2020-10-01 revision 5445e04352) [x86_64-linux]
```


## Building the Sorbet Compiler

Now that you have a VM archive available, you can build a version of Sorbet that
is able to generate native code.

```shell
$ bazel build //compiler:sorbet --config=release-linux
```

> NOTE: We're explicitly not passing the `--crosstool_top` flag for this build
> (like we did when building `sorbet_ruby`), as we want sorbet to us the version
> of Clang that we pin in the sandbox.

This will take quite a while, as it will build Sorbet and LLVM 15. If you've
ever been looking for an excuse to upgrade your development machine, this is it.
Once it's complete, you will find a binary in `bazel-bin/compiler/sorbet` that
behaves exactly like Sorbet the type checker, but has a few additional
command-line flags available. Copy it into the scratchpad as well:

```shell
$ mkdir -p scratchpad/bin
$ cp bazel-bin/compiler/sorbet scratchpad/bin
$ scratchpad/bin/sorbet --help
```

## Compiling some Ruby

Let's compile a simple program to start with:

```ruby
# typed: true
# frozen_string_literal: true
# compiled: true

xs = [1, 2, 3, 4]

xs.each do |x|
  puts "Element: #{x}"
end

puts xs
```

All of the sigils at the top of the file are required:

- The Sorbet Compiler requires that files are at least `typed: true` because
  otherwise Sorbet will not produce a typed CFG
- `frozen_string_literal: true` is required because of how we translate Ruby
  String literals (for simplicity, we have only implemented frozen string
  literals)
- `compiled: true` opts in to having the compiler emit a compiled artifact. (The
  compiler will not compile all files in a project by default.)

Place the Ruby program above in `test.rb`, and compile it with the following
commands:

```shell
$ mkdir compiled_output
$ mkdir ir_output
$ scratchpad/bin/sorbet --compiled-out-dir=compiled_output --llvm-ir-dir=ir_output test.rb
No errors! Great job.
$ ls compiled_output
test.rb.so
$ ls ir_output
test.rb.ll  test.rb.lowered.ll  test.rb.opt.ll
```

The files produced have suffixes that correspond to different phases of
compilation: `.ll` is the initial LLVM IR before any optimizations are applied;
`.lowered.ll` is that tree with some fast optimizations applied; `.opt.ll` is
the final version of the LLVM IR with all optimizations applied; `.rb.so` is the
native artifact that can be run by the Ruby VM.

## Running the compiled Ruby artifacts

You will need to patch `Kernel#require` in order to run the ruby code compiled
in the previous step. First, copy the `test/patch_require.rb` into the
`scratchpad` directory:

```
$ cp test/patch_require.rb scratchpad/
```

> If you'd like to read more about why `patch_require.rb` is necessary, the
> [compiler-caveats.md](compiler-caveats.md#runtime) document contains more of
> an explanation in the `Runtime` section.

Now, assuming that your generated code is in the `output` directory from the
previous step, you can use the following command to run the compiled code:

```
$ llvmir="./output" force_compile=true scratchpad/bin/ruby -r ./scratchpad/patch_require.rb -e 'require "test.rb"'
SorbetLLVM using compiled: ./output/test.rb.so for test.rb
Element: 1
Element: 2
Element: 3
Element: 4
1
2
3
4
```

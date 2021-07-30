# Running compiled code

The instructions will help you compile and run ruby using `sorbet_llvm` on linux.

## Building `sorbet_ruby`

`sorbet_llvm` relies on additional runtime support that we provide by linking it into the ruby vm. As a result, it is
not possible to run `sorbet_llvm` generated artifacts in a stock ruby vm. To build our custom vm, run the following
command:

```shell
$ bazel build @sorbet_ruby_2_7//:ruby.tar.gz \
    --config=release-linux \
    --crosstool_top=@bazel_tools//tools/cpp:toolchain
```

The flags that we give are:

* `--config=release-linux` - Build a release build, that will verify that `sorbet_llvm` artifacts were produced with the
  same revision of the compiler.
* `--crosstool_top=@bazel_tools//tools/cpp:toolchain` - Build using the native c++ toolchain instead of the one that we
  manage in the bazel sandbox. Without this flag the ruby build will be configured to use the c++ compiler that lives in
  the bazel sandbox to build extensions with native code components, which doesn't re-distribute well.

Once this build has finished, the archive `bazel-bin/external/sorbet_ruby_2_7/ruby.tar.gz` will contain the build of
ruby-2.7 with our patches applied. Copy that archive out of the build tree and extract it in a scratchpad directory:

```shell
$ mkdir scratchpad
$ cp bazel-bin/external/sorbet_ruby_2_7/ruby.tar.gz scratchpad
$ tar -xf scratchpad/ruby.tar.gz -C scratchpad
```

You can verify that it's doing the right thing by running:

```shell
$ scratchpad/bin/ruby --version
ruby 2.7.2p137 (2020-10-01 revision 5445e04352) [x86_64-linux]
```


## Building `sorbet_llvm`

Now that you have a vm archive available, you can build a version of sorbet that is able to generate native code:

```shell
$ bazel build //main:sorbet \
    --config=release-linux
```

> NOTE: We're explicitly not passing the `--crosstool_top` flag for this build, as we want sorbet to us the version of
> clang that we keep in the sandbox.

This will take quite a while, as it will build sorbet and LLVM-12. Once it's complete, you will find a binary in
`bazel-bin/main/sorbet` that behaves exactly as sorbet does from the open-source repo, but has a few additional
command-line flags available. Copy it into the scratchpad as well:

```shell
$ mkdir -p scratchpad/bin
$ cp bazel-bin/main/sorbet scratchpad/bin
$ scratchpad/bin/sorbet --help
```

## Compiling some ruby

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

All of the sigils at the top of the file are necessary:

* `sorbet_llvm` requires that files are at least `typed: true` because otherwise sorbet will not produce a CFG
* `frozen_string_literal: true` is required because of how we translate String constants
* and `compiled: true` is what allows us to toggle compilation at the file level

Place the ruby program above in `test.rb`, and compile it with the following commands:

```shell
$ mkdir compiled_output
$ mkdir ir_output
$ scratchpad/bin/sorbet --compiled-out-dir=compiled_output --llvm-ir-dir=ir_output test.rb
No errors! Great job.
$ ls compiled_output
test.rb.so
$ ls ir_output
test.rb.ll  test.rb.lll  test.rb.llo
```

The files produced have suffixes that correspond to different phases of compilation: `.lll` is the initial LLVM IR
before any optimizations are applied; `.ll` is that tree with some fast optimizations applied; `.llo` is the final
version of the LLVM IR with all optimizations applied; `.rb.so` is the native artifact that can be run by the ruby vm.

## Running the compiled ruby

You will need to patch `require` in order to run the ruby code compiled in the previous step. First, copy the
`test/patch_require.rb` into the `scratchpad` directory:

```
$ cp test/patch_require.rb scratchpad/
```

> If you'd like to read more about why `patch_require.rb` is necessary, the
> [compiler-caveats.md](compiler-caveats.md#runtime) document contains more of an explanation in the `Runtime` section.

Now, assuming that your generated code is in the `output` directory from the previous step, you can use the following
command to run the compiled code:

```
$ llvmir="./output" force_compile=true scratchpad/bin/ruby -r ./scratchpad/patch_require.rb -e 'require "test.rb"'
SorbetLLVM using compiled: ./output/test.rb.so for /pay/src/sorbet_llvm/test.rb
Element: 1
Element: 2
Element: 3
Element: 4
1
2
3
4
```

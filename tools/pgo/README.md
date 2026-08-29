# Profile-guided optimization

`linux-x86_64.profdata` is an LLVM instrumentation profile of `sorbet` typechecking a large production Ruby
codebase (about 600k files) with its usual flags, collected with `--config=pgo-instrument-linux`. It merges two
training runs, one with the original parser and one with `--parser=prism`, so both parsers are covered.

Build with it using `./bazel build //main:sorbet --config=pgo-linux`. Measured on that codebase, the profile-guided
binary is about 15% faster end to end and uses about 16% less CPU than `--config=release-linux`.

A stale profile is safe (clang ignores functions whose control flow changed, and warns about it only when asked),
but it helps less as the code drifts, so regenerate it periodically:

```
./bazel build //main:sorbet --config=pgo-instrument-linux
mkdir -p /tmp/pgo
LLVM_PROFILE_FILE=/tmp/pgo/sorbet-%p.profraw bazel-bin/main/sorbet <flags> <a big codebase>
LLVM_PROFILE_FILE=/tmp/pgo/sorbet-%p.profraw bazel-bin/main/sorbet <flags> --parser=prism <a big codebase>
bazel-sorbet/external/llvm_toolchain_15_0_7/bin/llvm-profdata merge -output=tools/pgo/linux-x86_64.profdata /tmp/pgo/*.profraw
```

The instrumented binary runs several times slower than a release build, so expect the training run to take a while.

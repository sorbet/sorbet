# Claude Code Guidelines for Sorbet

## Build System

- **NEVER use `bazel clean`** - The Sorbet build is expensive and takes a long time to rebuild from scratch. Incremental builds are preferred.

## Known Compiler Issues

The following tests have known issues that require investigation:

### vm_thread_mutex1/2/3 (test/testdata/ruby_benchmark/)
- **Issue**: Variable capture in nested blocks with Thread.new
- **Symptoms**: Variables captured from outer scope (like counter `r`) don't reflect modifications made inside Thread.new blocks
- **Root cause**: Likely related to how escaped variables are stored/accessed across thread boundaries in the compiler's closure implementation
- **Files involved**: compiler/IREmitter/IREmitterContext.cc (escape analysis), Payload.cc (variable get/set)

### lspace (test/testdata/ruby_benchmark/stripe/)
- **Issue**: Segfault at null function pointer (address 0xca)
- **Symptoms**: Crash when running compiled code
- **Possible causes**: Final method calls, attr_accessor on builtin Exception class, or complex inheritance patterns

# Claude Code Guidelines for Sorbet

## Build System

- **NEVER use `bazel clean`** - The Sorbet build is expensive and takes a long time to rebuild from scratch. Incremental builds are preferred.

# AGENTS.md

This file provides guidance to agentic AI assistants when working with code in this repository.

## Build Commands

```bash
# Standard development build
./bazel build //main:sorbet --config=dbg

# Run the built binary
bazel-bin/main/sorbet -e "1 + false"
bazel-bin/main/sorbet foo.rb

# Run all tests
bazel test //... --config=dbg

# Run curated fast subset of tests (preferred during development)
bazel test test --config=dbg

# Run tests with output on failure
bazel test //... --config=dbg --test_output=errors

# Run a single test
bazel test //test:test_PosTests/testdata/path/to/<name>
bazel test //test:test_LSPTests/testdata/lsp/<name>

# Format C++ files
tools/scripts/format_cxx.sh

# Format Bazel BUILD files
tools/scripts/format_build_files.sh

# Generate compile_commands.json for editor tooling
tools/scripts/build_compilation_db.sh

# Create a local bazelrc with caching enabled
tools/create_local_bazelrc.sh
```

## Updating Test Snapshots

```bash
# Update all snapshot files (exp, out, etc.)
tools/scripts/update_exp_files.sh

# Update only *.exp files in test/testdata
tools/scripts/update_testdata_exp.sh

# Update only *.exp files in a subdirectory
tools/scripts/update_testdata_exp.sh test/testdata/cfg

# Update only a single test's exp files
tools/scripts/update_testdata_exp.sh test/testdata/cfg/next.rb

# Update CLI test .out files
bazel test //test/cli:update
```

## Build Configurations

- `--config=dbg` — Standard development build; good stack traces, all ENFORCEs enabled
- `--config=sanitize` — UBSan + ASan; for memory/UB errors (slower, larger)
- `--config=static-libs` — Force static linking (better debug symbols with lldb)
- `-c opt` — Enable clang optimizations (`-O2`); can be combined with any config

## Architecture

Sorbet is a C++ type checker for Ruby built with Bazel. The codebase follows a **multi-phase pipeline** architecture.

### Pipeline Overview

The pipeline has three major stages:

1. **Index** (per-file, parallelizable, cacheable): Parse → Desugar → Rewriter → LocalVars
2. **nameAndResolve** (whole-program, partially parallel): Namer → Resolver
3. **typecheck** (per-method, parallelizable): DefinitionValidator → CFG → Infer

### Phases (in order)

| Phase | Folder | IR In → IR Out | Key job |
|-------|--------|----------------|---------|
| Parser | `parser/` | source → `parser::Node` | Parse Ruby (whitequark or Prism) |
| Desugar | `ast/desugar/` | `parser::Node` → `ast::ExpressionPtr` | Reduce ~127 parser node types to ~31 AST types; canonicalize syntax |
| Rewriter | `rewriter/` | `ast::ExpressionPtr` → `ast::ExpressionPtr` | Expand DSLs (`attr_reader`, etc.) |
| LocalVars | `local_vars/` | `ast::ExpressionPtr` → `ast::ExpressionPtr` | Resolve local variable identifiers |
| Namer | `namer/` | `ast::ExpressionPtr` → `ast::ExpressionPtr` | Create `Symbol`s for classes/methods/args in `GlobalState` |
| Resolver | `resolver/` | `ast::ExpressionPtr` → `ast::ExpressionPtr` | Resolve constants; fill in type info on `Symbol`s |
| DefinitionValidator | `definition_validator/` | `ast::ExpressionPtr` | Emit errors about invalid definitions |
| CFG | `cfg/` | `ast::ExpressionPtr` → `cfg::CFG` | Build control flow graph (basic blocks) |
| Infer | `infer/` | `cfg::CFG` | Type-check method bodies; annotate bindings with types |

Use `-p <phase>` (e.g., `-p desugar-tree`, `-p symbol-table`, `-p cfg`) to print the IR at any phase. Use `--stop-after <phase>` to halt early.

### Core Data Structures (`core/`)

- **`GlobalState`** — Central store for all `Symbol`s, `Name`s, `File`s. Mutable during index/resolve; frozen before typecheck.
- **`Symbol` / `SymbolRef`** — Semantic info about definitions (classes, methods, fields, args). `SymbolRef` is a compact handle; dereference with `.data(gs)`.
- **`Name` / `NameRef`** — Interned strings. Two `NameRef`s with the same ID are equal; comparison is fast.
- **`File` / `FileRef`** — Source files tracked in GlobalState.
- **`core::Loc`** — Source location (pronounced "lohk"). Compact bit-packed representation. Use `beginError` which lazily builds error messages only if they'll be reported.
- **`ast::ExpressionPtr`** — AST nodes using tagged pointers; avoid virtual dispatch. Use `typecase` for pattern matching over node types.
- **`cfg::CFG`** — Control flow graph: a vector of `BasicBlock`s, each containing `Binding`s (instruction → local variable assignment). Not SSA.

### Type System (`core/types/`)

- Types live in `core/Types.h`; method call checking is in `core/types/calls.cc`
- Inference is single-pass forward (processes CFG in topological order; no backsolving)
- `lub` = type union ("or"), `glb` = type intersection ("and")
- Intrinsics: methods whose return types are computed in C++ rather than via sigs

### LSP (`main/lsp/`)

The LSP server reuses the same pipeline with incremental updates. File changes take either a **fast path** (local changes, re-typecheck affected files) or **slow path** (structural changes, re-run nameAndResolve). See `incrementalResolve` in the codebase.

### Other Key Directories

- `main/pipeline/` — Sequences the phases end-to-end
- `main/options/` — CLI option parsing
- `rbi/` — Ruby Interface files (stdlib type definitions)
- `gems/sorbet/` — Ruby gem for `srb init`
- `gems/sorbet-runtime/` — Ruby gem for runtime type checks
- `test/testdata/` — test_corpus tests (`.rb` files with `# error:` annotations)
- `test/cli/` — CLI tests (`test.sh` + `test.out`)
- `test/testdata/lsp/` — LSP tests with `def:`, `usage:`, `hover:`, `completion:` annotations
- `parser/` — Ruby parsers: `parser/parser/` (whitequark-derived yacc/C++), `parser/prism/` (Prism-based)

## Writing Tests

**Preferred test types (most to least preferred):**

1. **test_corpus** (`test/testdata/`): Add a `.rb` file. Mark expected errors with `# error: <message>`. Run with `bazel test //test:test_PosTests/testdata/<path>`.

2. **Expectation tests**: Add `<name>.rb.<phase>.exp` alongside a testdata file. Must exactly match `sorbet -p <phase> <name>.rb`.

3. **LSP tests** (`test/testdata/lsp/`): Use annotations like `def:`, `usage:`, `hover:`, `completion:`, `apply-completion:`, `apply-code-action:`, `apply-rename:`, `symbol-search:`, `type:`, `type-def:`, `find-implementation:`, `implementation:`, `show-symbol:` (see `test/helpers/position_assertions.cc` for the full list). Run with `bazel test //test:test_LSPTests/testdata/lsp/<name>`.

4. **CLI tests** (`test/cli/<name>/`): Add `test.sh` (executable) + `test.out`. Run with `bazel test //test/cli:test_<name>`.

5. **LSP recorded tests** (`test/lsp/`): Avoid; prefer regular LSP tests.

**Error annotation syntax:**
```ruby
1 + '' # error: `String` doesn't match `Integer`

rescue Foo, Bar => baz
     # ^^^ error: Unable to resolve constant `Foo`
          # ^^^ error: Unable to resolve constant `Bar`
```

**Multi-file tests**: Files sharing a `foo__` prefix (e.g., `foo__1.rb`, `foo__2.rb`) are tested together as test `foo`.

## Debugging

```bash
# Debug with lldb
lldb bazel-bin/main/sorbet -- <args>
# (use --config=static-libs for better symbols)

# Attach to running LSP process
# Launch sorbet with --wait-for-dbg, then:
lldb -p <pid>

# Render CFG as SVG (requires graphviz)
tools/scripts/cfg-view.sh -e 'while true; puts 42; end'
```

Best practice: add an `ENFORCE` (assertion) that catches the bug before fixing it. `ENFORCE`s are compiled out in release builds.

## Useful Flags for Development

- `-p <IR>` — Print internal representation at a phase (see `--help` for all options)
- `--stop-after <phase>` — Stop pipeline early
- `-v`, `-vv`, `-vvv` — Increase logger verbosity
- `--max-threads=1` — Disable parallelism (helps isolate concurrency bugs)
- `--wait-for-dbg` — Pause on startup for debugger to attach
- `--no-stdlib` — Skip loading stdlib RBIs (faster for isolated tests)

## C++ Patterns

- **`typecase`**: Pattern matching over tagged-pointer AST/instruction types. Heavily used in desugarer, rewriter, and infer passes.
- **`show` vs `toString`**: `show` = user-visible string (like `Display`); `toString` = debug/internal string (like `Debug`).
- **`ENFORCE`**: In-line assertion (like `assert`); only runs in debug builds.
- **`sanityCheck`**: Consistency check called at phase boundaries; only in debug builds.
- Code is C++17; uses Abseil and a custom thread pool for parallelism.

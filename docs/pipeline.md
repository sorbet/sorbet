# pipeline

This is a high level overview of Sorbet's pipeline.

It's best understood with [Internals.md](internals.md) open in another tab,
where what the phases of Sorbet actually do are documented.

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                                      index                                       │
├───────────────────────────┬───────────────────────────┬──────────────────────────┤
│ ┌───────────┐             │┌───────────┐              │┌───────────┐             │
│ │GlobalState│             ││GlobalState│              ││GlobalState│             │
│ └───────────┘             │└───────────┘              │└───────────┘             │
│                           │                           │                          │
│       ┌──────────┐        │       ┌──────────┐        │      ┌──────────┐        │
│       │ one file │        │       │ one file │        │      │ one file │        │
│       └──────────┘        │       └──────────┘        │      └──────────┘        │
│             │             │             │             │            │             │
│             │ parser      │             │ parser      │            │ parser      │
│             ▼             │             ▼             │            ▼             │
│     ┌──────────────┐      │     ┌──────────────┐      │    ┌──────────────┐      │
│     │ parser::Node │      │     │ parser::Node │      │    │ parser::Node │      │
│     └──────────────┘      │     └──────────────┘      │    └──────────────┘      │
│             │             │             │             │            │             │
│             │ desugarer   │             │ desugarer   │            │ desugarer   │
│             ▼             │             ▼             │            ▼             │
│    ┌─────────────────┐    │    ┌─────────────────┐    │   ┌─────────────────┐    │
│    │ ast::Expression │    │    │ ast::Expression │    │   │ ast::Expression │    │
│    └─────────────────┘    │    └─────────────────┘    │   └─────────────────┘    │
│             │             │             │             │            │             │
│             │ rewriter         │             │ rewriter         │            │ rewriter         │
│             ▼             │             ▼             │            ▼             │
│    ┌─────────────────┐    │    ┌─────────────────┐    │   ┌─────────────────┐    │
│    │ ast::Expression │    │    │ ast::Expression │    │   │ ast::Expression │    │
│    └─────────────────┘    │    └─────────────────┘    │   └─────────────────┘    │
│             │             │             │             │            │             │
│             │ local_vars  │             │ local_vars  │            │ local_vars  │
│             ▼             │             ▼             │            ▼             │
│    ┌─────────────────┐    │    ┌─────────────────┐    │   ┌─────────────────┐    │
│    │ ast::Expression │    │    │ ast::Expression │    │   │ ast::Expression │    │
│    └─────────────────┘    │    └─────────────────┘    │   └─────────────────┘    │
│                           │                           │                          │
└───────────────────────────┴───────────────────────────┴──────────────────────────┘
              │                           │                          │
              │                           │                          │
              └───────────────────────────┼──────────────────────────┘
                                          │
                                          │  merge Index results
                                          ▼
            ┌──────────────────────────────────────────────────────────┐
            │  ┌───────────────────┐                                   │
            │  │    GlobalState    │  resolve                          │
            │  └───────────────────┘                                   │
            ├──────────────────────────────────────────────────────────┤
            │                   ┌───────────────────┐                  │
            │                   │  ast::Expression  │                  │
            │                   └───────────────────┘                  │
            │                             │                            │
            │                             │ namer                      │
            │                             ▼                            │
            │                   ┌───────────────────┐                  │
            │                   │  ast::Expression  │                  │
            │                   └───────────────────┘                  │
            │                             │                            │
            │                             │ resolver                   │
            │                             ▼                            │
            │                   ┌───────────────────┐                  │
            │                   │  ast::Expression  │                  │
            │                   └───────────────────┘                  │
            │                                                          │
            └──────────────────────────────────────────────────────────┘
                                          │
                                          │  freeze GlobalState
                                          ▼
 ┌────────────────────────────────────────────────────────────────────────────────┐
 │  ┌───────────────────┐                                                         │
 │  │    GlobalState    │            typecheck                                    │
 │  └───────────────────┘                                                         │
 ├──────────────────────────┬──────────────────────────┬──────────────────────────┤
 │                          │                          │                          │
 │  ┌───────────────────┐   │   ┌───────────────────┐  │   ┌───────────────────┐  │
 │  │  ast::Expression  │   │   │  ast::Expression  │  │   │  ast::Expression  │  │
 │  └───────────────────┘   │   └───────────────────┘  │   └───────────────────┘  │
 │            │             │             │            │             │            │
 │            │ cfg         │             │ cfg        │             │ cfg        │
 │            ▼             │             ▼            │             ▼            │
 │      ┌──────────┐        │       ┌──────────┐       │       ┌──────────┐       │
 │      │ cfg::CFG │        │       │ cfg::CFG │       │       │ cfg::CFG │       │
 │      └──────────┘        │       └──────────┘       │       └──────────┘       │
 │            │             │             │            │             │            │
 │            │ infer       │             │ infer      │             │ infer      │
 │            ▼             │             ▼            │             ▼            │
 │   ┌─────────────────┐    │    ┌─────────────────┐   │    ┌─────────────────┐   │
 │   │    cfg::CFG     │    │    │    cfg::CFG     │   │    │    cfg::CFG     │   │
 │   │  (with types)   │    │    │  (with types)   │   │    │  (with types)   │   │
 │   └─────────────────┘    │    └─────────────────┘   │    └─────────────────┘   │
 │                          │                          │                          │
 └──────────────────────────┴──────────────────────────┴──────────────────────────┘
```

Some notes:

- The pipeline is broken up (in the code, but importantly in the way we talk
  about it amongst ourselves) into three big chunks:

  1.  index
  2.  resolve
  3.  typecheck

- The "index" chunk works at the file level and involves largely syntactic
  transformations (though it does populate some information into GlobalState).
  As a result, we can cache the output of indexing a single file to skip
  indexing that file entirely if it's contents haven't changed.

  This also means we can parallelize indexing files, represented in the diagram
  below by multiple columns (parallel execution contexts). And this also means
  this phase is trivially incremental (for LSP).

- After indexing each file, there will be multiple GlobalState's, which might
  disagree on IDs used to represent `NameRef`s entered in parallel. To fix this
  up and arrive at one GlobalState, we merge all indexing results.

- The "resolve" chunk is a misleading name because it's composed of both Namer
  and Resolver. It's largely sequential, though there is a little bit of
  parallelism (see the implementation of Resolver).

  Whereas indexing is largely concerned with syntax transformations (entering
  some information into GlobalState), it's the opposite for the "resolve" chunk:
  Namer and Resolver are largely concerned with populating GlobalState,
  specifically the symbol table, but they also do some light syntax
  modifications.

  This chunk is a little trickier to be made incremental—sometimes we can, but
  sometimes respond to an update we have to re-do this chunk entirely. We call
  these "the LSP fast path" and "the LSP slow path." The details of when take
  which path are a bit hairy, but at a high level it's a function of just how
  different GlobalState is after an incremental resolve.

- The "typecheck" chunk operates on an immutable GlobalState. And because we
  don't do global type inference, type checking one method can't affect the
  result of type checking another method, so we can type check individual
  methods in parallel (again, represented with multiple columns).

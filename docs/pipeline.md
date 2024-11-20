# pipeline

This is a high level overview of Sorbet's pipeline.

It's best understood with [Internals.md](internals.md) open in another tab,
where what the phases of Sorbet actually do are documented.

```
┌───────────────────────────────────────────────────────────────────────────────────┐
│                                       index                                       │
├───────────────────────────┬───────────────────────────┬───────────────────────────┤
│ ┌───────────┐             │ ┌───────────┐             │ ┌───────────┐             │
│ │GlobalState│             │ │GlobalState│             │ │GlobalState│             │
│ └───────────┘             │ └───────────┘             │ └───────────┘             │
│                           │                           │                           │
│       ┌──────────┐        │       ┌──────────┐        │       ┌──────────┐        │
│       │ one file │        │       │ one file │        │       │ one file │        │
│       └──────────┘        │       └──────────┘        │       └──────────┘        │
│             │             │             │             │             │             │
│             │ parser      │             │ parser      │             │ parser      │
│             ▼             │             ▼             │             ▼             │
│     ┌──────────────┐      │     ┌──────────────┐      │     ┌──────────────┐      │
│     │ parser::Node │      │     │ parser::Node │      │     │ parser::Node │      │
│     └──────────────┘      │     └──────────────┘      │     └──────────────┘      │
│             │             │             │             │             │             │
│             │ desugarer   │             │ desugarer   │             │ desugarer   │
│             ▼             │             ▼             │             ▼             │
│    ┌─────────────────┐    │    ┌─────────────────┐    │    ┌─────────────────┐    │
│    │ ast::Expression │    │    │ ast::Expression │    │    │ ast::Expression │    │
│    └─────────────────┘    │    └─────────────────┘    │    └─────────────────┘    │
│             │             │             │             │             │             │
│             │ rewriter    │             │ rewriter    │             │ rewriter    │
│             ▼             │             ▼             │             ▼             │
│    ┌─────────────────┐    │    ┌─────────────────┐    │    ┌─────────────────┐    │
│    │ ast::Expression │    │    │ ast::Expression │    │    │ ast::Expression │    │
│    └─────────────────┘    │    └─────────────────┘    │    └─────────────────┘    │
│             │             │             │             │             │             │
│             │ local_var   │             │ local_var   │             │ local_var   │
│             ▼             │             ▼             │             ▼             │
│    ┌─────────────────┐    │    ┌─────────────────┐    │    ┌─────────────────┐    │
│    │ ast::Expression │    │    │ ast::Expression │    │    │ ast::Expression │    │
│    └─────────────────┘    │    └─────────────────┘    │    └─────────────────┘    │
└───────────────────────────┴───────────────────────────┴───────────────────────────┘
              │                           │                           │
              │                           │                           │
              └───────────────────────────┼───────────────────────────┘
                                          │
                                          │ merge index results
                                          ▼
             ┌─────────────────────────────────────────────────────────┐
             │  ┌─────────────┐                                        │
             │  │ GlobalState │    nameAndResolve                      │
             │  └─────────────┘                                        │
             ├─────────────────────────────────────────────────────────┤
             │                   ┌─────────────────┐                   │
             │                   │ ast::Expression │                   │
             │                   └─────────────────┘                   │
             │                        ┌───┼───┐                        │
             │                        ▼   ▼   ▼                        │
             │                        └───┼───┘                        │
             │                            ▼       namer                │
             │                        ┌───┼───┐                        │
             │                        ▼   ▼   ▼                        │
             │                        └───┼───┘                        │
             │                            ▼                            │
             │                   ┌─────────────────┐                   │
             │                   │ ast::Expression │                   │
             │                   └─────────────────┘                   │
             │                        ┌───┼───┐                        │
             │                        ▼   ▼   ▼                        │
             │                        └───┼───┘                        │
             │                            ▼                            │
             │                            │       resolver             │
             │                            ▼                            │
             │                            │                            │
             │                            ▼                            │
             │                   ┌─────────────────┐                   │
             │                   │ ast::Expression │                   │
             │                   └─────────────────┘                   │
             └─────────────────────────────────────────────────────────┘
                                          │
                                          │ freeze GlobalState
                                          ▼
┌───────────────────────────────────────────────────────────────────────────────────┐
│  ┌─────────────┐                                                                  │
│  │ GlobalState │                    typecheck                                     │
│  └─────────────┘                                                                  │
├───────────────────────────┬───────────────────────────┬───────────────────────────┤
│  ┌─────────────────┐      │  ┌─────────────────┐      │  ┌─────────────────┐      │
│  │ ast::Expression │      │  │ ast::Expression │      │  │ ast::Expression │      │
│  └─────────────────┘      │  └─────────────────┘      │  └─────────────────┘      │
│  │                        │  │                        │  │                        │
│  │ definition_validator   │  │ definition_validator   │  │ definition_validator   │
│  ▼                        │  ▼                        │  ▼                        │
│  ┌─────────────────┐      │  ┌─────────────────┐      │  ┌─────────────────┐      │
│  │ ast::Expression │      │  │ ast::Expression │      │  │ ast::Expression │      │
│  └─────────────────┘      │  └─────────────────┘      │  └─────────────────┘      │
│           │               │           │               │           │               │
│           │ class_flatten │           │ class_flatten │           │ class_flatten │
│           ▼               │           ▼               │           ▼               │
│  ┌─────────────────┐      │  ┌─────────────────┐      │  ┌─────────────────┐      │
│  │ ast::Expression │      │  │ ast::Expression │      │  │ ast::Expression │      │
│  └─────────────────┘      │  └─────────────────┘      │  └─────────────────┘      │
│           │               │           │               │           │               │
│           │ cfg           │           │ cfg           │           │ cfg           │
│           ▼               │           ▼               │           ▼               │
│     ┌──────────┐          │     ┌──────────┐          │     ┌──────────┐          │
│     │ cfg::CFG │          │     │ cfg::CFG │          │     │ cfg::CFG │          │
│     └──────────┘          │     └──────────┘          │     └──────────┘          │
│           │               │           │               │           │               │
│           │ infer         │           │ infer         │           │ infer         │
│           ▼               │           ▼               │           ▼               │
│   ┌──────────────┐        │   ┌──────────────┐        │   ┌──────────────┐        │
│   │   cfg::CFG   │        │   │   cfg::CFG   │        │   │   cfg::CFG   │        │
│   │ (with types) │        │   │ (with types) │        │   │ (with types) │        │
│   └──────────────┘        │   └──────────────┘        │   └──────────────┘        │
│                           │                           │                           │
└───────────────────────────┴───────────────────────────┴───────────────────────────┘
```

Some notes:

- The pipeline is broken up (in the code, but importantly in the way we talk
  about it amongst ourselves) into three big chunks:

  1.  index
  2.  nameAndResolve
  3.  typecheck

- The "index" chunk works at the file level and involves largely syntactic
  transformations (though it does populate some information into GlobalState).
  As a result, we can cache the output of indexing a single file to skip
  indexing that file entirely if its contents haven't changed.

  This also means we can trivially parallelize indexing files, represented in
  the diagram below by multiple columns (parallel execution contexts). And this
  also means this phase is trivially incremental (for LSP).

- After indexing each file, there will be multiple GlobalState's, which might
  disagree on IDs used to represent interned strings (`NameRef`s) entered in
  parallel. To fix this up and arrive at one where each interned string has one
  ID in GlobalState, we merge all indexing results.

- The "resolve" chunk is a misleading name because it's composed of both Namer
  and Resolver. At this point, both those names are also misleading because
  they're implemented by things which arguably look like mini phases in their
  own right. Since this is an overview, I've only listed `namer` and `resolver`
  as the two phases that make up the middle `resolve` box, but I've drawn a
  bunch of arrows to hint that there's more going on.

  Whereas indexing is largely concerned with syntax transformations (entering
  some information into GlobalState), it's the opposite for the "resolve" chunk:
  this phase is largely concerned with populating GlobalState (specifically the
  symbol table), but they also do some light syntax modifications.

  This whole phase used to be entirely sequential and whole program, but over
  time parts of it have become parallel and incremental. That being said, we've
  tried to parallelize and incrementalize it in a way that makes reasoning about
  it as if it were sequential and whole program still valid. At this point,
  almost all of Namer is parallel via multiple fork/joins, and one particular
  part of Resolver is parallel, again with a fork/join.

  For more information on what's parallelized, see [Namer & Resolver
  Pipeline](namer-resolver-pipeline.md).

  The sequential parts of nameAndResolve were originally designed to be
  whole-program, which makes it tricky to make incremental. Our current trick is
  to separate changes into "local changes" (fast path) and "non-local changes"
  (slow path). It's easy to run nameAndResolve in a mode where it assumes it's
  on the fast path but checks to see if it needs to take the slow path. The
  details are a bit hairy, but you can see them if you look for
  `incrementalResolve` in the code.

- The "typecheck" chunk operates on an immutable GlobalState. Because we don't
  do global type inference, type checking one method can't affect the result of
  type checking another method, so we can type check individual Ruby methods by
  running the individual phases of `typecheck` completely in parallel on a
  single method (again, this parallelism is represented with multiple columns).

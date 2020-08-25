# Namer & Resolver Pipeline

This is a zoomed in view of the parallelism employed by the Namer and Resolver
phases of Sorbet.

For a fuller picture of Sorbet's pipeline, see [pipeline.md](pipeline.md).

```
            ┌─────────────────────────────────────────────────────────────────────────────────────────────────┐
            │                                              namer                                              │
            ├─────────────────────────────────────────────────────────────────────────────────────────────────┤
            │       ┌─────────────────┐             ┌─────────────────┐             ┌─────────────────┐       │
            │       │ ast::Expression │             │ ast::Expression │             │ ast::Expression │       │
            │       └─────────────────┘             └─────────────────┘             └─────────────────┘       │
            │                │                               │                               │                │
            │                │ findSymbols                   │ findSymbols                   │ findSymbols    │
            │                ▼                               ▼                               ▼                │
            │   ┌─────────────────────────┐     ┌─────────────────────────┐     ┌─────────────────────────┐   │
            │   │ namer::FoundDefinitions │     │ namer::FoundDefinitions │     │ namer::FoundDefinitions │   │
            │   └─────────────────────────┘     └─────────────────────────┘     └─────────────────────────┘   │
            │                │                               │                               │                │
            │                │                               │                               │                │
            │                └───────────────────────────────┼───────────────────────────────┘                │
            │                                                │                                                │
            │                                                │ merge findSymbols results                      │
            │                                                ▼                                                │
            │                                   ┌─────────────────────────┐                                   │
            │                                   │ namer::FoundDefinitions │                                   │
            │                                   └─────────────────────────┘                                   │
            │                                                │                                                │
            │                                                │ defineSymbols                                  │
            │                                                ■                                                │
            │                                                  (mutates GlobalState)                          │
            │                                                                                                 │
            │                                                                                                 │
            │       ┌─────────────────┐             ┌─────────────────┐             ┌─────────────────┐       │
            │       │ ast::Expression │             │ ast::Expression │             │ ast::Expression │       │
            │       └─────────────────┘             └─────────────────┘             └─────────────────┘       │
            │                │                               │                               │                │
            │                │ symbolizeTrees                │ symbolizeTrees                │ symbolizeTrees │
            │                ▼                               ▼                               ▼                │
            │       ┌─────────────────┐             ┌─────────────────┐             ┌─────────────────┐       │
            │       │ ast::Expression │             │ ast::Expression │             │ ast::Expression │       │
            │       └─────────────────┘             └─────────────────┘             └─────────────────┘       │
            └─────────────────────────────────────────────────────────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                        resolver                                                         │
├─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│ ┌─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐ │
│ │                                                  resolveConstants                                                   │ │
│ ├─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤ │
│ │                 ┌─────────────────┐             ┌─────────────────┐                ┌─────────────────┐              │ │
│ │                 │ ast::Expression │             │ ast::Expression │                │ ast::Expression │              │ │
│ │                 └─────────────────┘             └─────────────────┘                └─────────────────┘              │ │
│ │                          │                               │                                  │                       │ │
│ │                          │ ResolveConstantsWalk          │ ResolveConstantsWalk             │ ResolveConstantsWalk  │ │
│ │                          ▼                               ▼                                  ▼                       │ │
│ │              ┌───────────────────────┐       ┌───────────────────────┐          ┌───────────────────────┐           │ │
│ │              │ resolver "todo" lists │       │ resolver "todo" lists │          │ resolver "todo" lists │           │ │
│ │              └───────────────────────┘       └───────────────────────┘          └───────────────────────┘           │ │
│ │                          │                               │                                  │                       │ │
│ │                          │                               │                                  │                       │ │
│ │                          └───────────────────────────────┼──────────────────────────────────┘                       │ │
│ │                                                          │                                                          │ │
│ │                                                          │ merge "todo" lists                                       │ │
│ │                                                          ▼                                                          │ │
│ │                                              ┌───────────────────────┐                                              │ │
│ │                                              │ resolver "todo" lists │                                              │ │
│ │                                              └───────────────────────┘                                              │ │
│ │                                                          │                                                          │ │
│ │                                                          │ resolveJob, et al.                                       │ │
│ │                                                          ■                                                          │ │
│ │                                                                                                                     │ │
│ └─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘ │
│                                                            │                                                            │
│                                                            │                                                            │
│                                                            ▼                                                            │
│                                                   ┌─────────────────┐                                                   │
│                                                   │ ast::Expression │                                                   │
│                                                   └─────────────────┘                                                   │
│                                                            │                                                            │
│                                                            │ finalizeAncestors                                          │
│                                                            ▼                                                            │
│                                                   ┌─────────────────┐                                                   │
│                                                   │ ast::Expression │                                                   │
│                                                   └─────────────────┘                                                   │
│                                                            │                                                            │
│                                                            │ resolveMixesInClassMethods                                 │
│                                                            ▼                                                            │
│                                                   ┌─────────────────┐                                                   │
│                                                   │ ast::Expression │                                                   │
│                                                   └─────────────────┘                                                   │
│                                                            │                                                            │
│                                                            │ ResolveTypeMembersWalk                                     │
│                                                            ▼                                                            │
│                                                   ┌─────────────────┐                                                   │
│                                                   │ ast::Expression │                                                   │
│                                                   └─────────────────┘                                                   │
│                                                            │                                                            │
│                                                            │ resolveSigs                                                │
│                                                            ▼                                                            │
│                                                   ┌─────────────────┐                                                   │
│                                                   │ ast::Expression │                                                   │
│                                                   └─────────────────┘                                                   │
└─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

Some notes:

- Namer uses two major parallel parts and one sequential part.

  The first parallel part of namer walks all the `ast::Expression`s in parallel
  by file, and collects essentially a list of "definitions (`Symbol`s) that will
  need to be entered into GlobalState for this file" but doesn't enter them.

  Then it merges all these lists into one big list, and runs a sequential loop
  over the would-be-definitions (`namer::FoundDefinitions`) and actually enters
  them into GlobalState. We do this to avoid having to deal with concurrent
  writes to `GlobalState` using e.g. some sort of locking protocol.

  With GlobalState updated, Namer walks all the `ast::Expression`s in parallel
  again, this time annotating each AST node with the Symbol that was just
  entered into GlobalState for it. Specifically, this only annotates the
  definition AST nodes (class defs, method defs, etc.). It doesn't annotate any
  of the usage sites with their symbols—that's (one of) Resolver's job(s).

- Only one thing in Resolver is parallelized: discovering constant literal
  usages. It's similar to Namer: the found definitions are accumulated into
  various kinds of "todo" lists. These "todo" lists are then consumed in a
  sequential fixed point until there's nothing left to do as far as constant
  resolution.

  The other parts of Resolver are not parallel. This is largely because they all
  mutate GlobalState, and again, we don't have any concurrent mutable access to
  GlobalState in Sorbet.

  It's possible that these parts of Resolver could be parallelized in the
  future if we decide that they're too slow.

- In general, we have to be very careful when considering adding features to
  Resolver. Most of Resolver is single-threaded and whole-program. That means
  potentially doing a lot of work and making Sorbet very slow. We try to put as
  little work in Resolver as possible.

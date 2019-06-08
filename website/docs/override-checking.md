---
id: override-checking
title: Override Checking
---

Sorbet supports method override checking. These checks are implemented as
`sig` annotations:

- `overridable` means children can override this method
- `override` means this method overrides a method on its parent (or ancestor)
- `abstract` means this method is abstract (has no implementation) and must be
  implemented by being overridden in all concrete subclasses.
- `implementation` means this method implements an abstract method

These annotations can be chained, for example `.implementation.overridable`
lets a grandchild class override a concrete implementation of its parent.

Use this table to track when annotations can be used, although the error
messages are the canonical source of truth. ✅ means "this pairing is allowed"
while ❌ means "this is an error" (currently, most of these errors are runtime
errors, though in the future they may become static errors).

> Below, `standard` (for the child or parent) means "has a `sig`, but has none
> of the special modifiers."

| ↓Parent \ Child → | no sig | `standard` | `override` | `implementation` |
| ----------------- | :----: | :--------: | :--------: | :--------------: |
| no sig            | ✅     | ✅         | ✅         | ❌               |
| `standard`        | ✅     | ✅         | ❌         | ❌               |
| `overridable`     | ✅     | ❌         | ✅         | ❌               |
| `override`        | ✅     | ❌         | ✅         | ❌               |
| `implementation`  | ✅     | ❌         | ❌         | ❌               |
| `abstract`        | ✅     | ❌         | ❌         | ✅               |

> **Note**: if the implementation methods are inherited--from either a class or
> mixin--the methods don't need the `implementation` annotation.


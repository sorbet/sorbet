---
id: stdlib-generics
title: Generics in the Standard Library
sidebar_label: Builtin Generics
---

> TODO(jez) This page is still a fragment. Contributions welcome!

Sorbet supports [generics types](generics.md). The syntax looks likes
`MyClass[Elem]`. For user defined generic classes, it's possible to make this
valid Ruby syntax.

However, it's not possible to change the syntax for classes in the Ruby standard
library that should be generic. To make up for this, we use wrappers in the T::
namespace to represent them:

- `T::Array[Type]`
  - This validates that the value is an array of any length where all elements
    of the array are the specified type.
- `T::Hash[KeyType, ValueType]`
  - This validates that the value is a hash where all keys of the type specified
    by `KeyType`, and all values are of the type specified by `ValueType`.
- `T::Set[Type]`
- `T::Enumerable[Type]`
- `T::Range[Type]`

While you still can write Array or Hash inside a sig, they will be treated as
`T::Array[T.untyped]` and `T::Hash[T.untyped, T.untyped]` respectively.

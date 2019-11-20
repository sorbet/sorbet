---
id: stdlib-generics
title: Arrays, Hashes, and Generics in the Standard Library
sidebar_label: Arrays & Hashes
---

> TODO: This page is still a fragment. Contributions welcome!

Sorbet supports generic types. The syntax looks likes `MyClass[Elem]`. For user
defined generic classes, it's possible to make this valid Ruby syntax.

However, it's not possible to change the syntax for classes in the Ruby standard
library that should be generic. To make up for this, we use wrappers in the `T`
namespace to represent them:

- `T::Array[Type]`
- `T::Hash[KeyType, ValueType]`
- `T::Set[Type]`
- `T::Enumerable[Type]`
- `T::Enumerator[Type]`
- `T::Range[Type]`

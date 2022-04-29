---
id: stdlib-generics
title: Arrays, Hashes, and Generics in the Standard Library
sidebar_label: Arrays & Hashes
---

The Sorbet syntax for type annotations representing arrays, hash maps, and other
containers defined in the Ruby standard library looks different from other
[class types](class-types.md) despite the fact that Ruby uses classes to
represent these values, too. Here's the syntax Sorbet uses:

| Type                           | Example value                         |
| ------------------------------ | ------------------------------------- |
| `T::Array[Integer]`            | `[1, 2, 3]`                           |
| `T::Array[String]`             | `["hello", "goodbye"]`                |
| `T::Hash[Symbol, Integer]`     | `{key: 0}`                            |
| `T::Hash[String, Float]`       | `{"key" => 0.0}`                      |
| `T::Set[Integer]`              | `Set[1, 2, 3]`                        |
| `T::Range[Integer]`            | `0..10`                               |
| `T::Enumerable[Integer]`       | _interface implemented by many types_ |
| `T::Enumerator[Integer]`       | [1, 2, 3].each                        |
| `T::Enumerator::Lazy[Integer]` | [1, 2, 3].each.lazy                   |

## Why the `T::` prefix?

Sorbet uses syntax like `MyClass[Elem]` for type arguments passed to
[generic classes](generics.md). All Sorbet type annotations are backwards
compatible with normal Ruby syntax, and this is no exception. In normal Ruby,
`MyClass[Elem]` would correspond to a call to a method named `[]` defined on
`MyClass`.

When creating user-defined generic classes, the `sorbet-runtime` gem
automatically defines this method so that the type annotation syntax works at
runtime.

But for classes in the Ruby standard library that Sorbet retroactively defined
as generic classes, the `[]` method will not always be defined at runtime. One
potential option would have been to use `sorbet-runtime` to monkey patch the
standard library so that the `[]` method is defined for generic classes, but
some of these Ruby standard library classes **already** define a meaningful `[]`
method. For example:

```ruby
Array[1, 2, 3]
# => evaluates to the array `[1, 2, 3]`

Set[1, 2, 3]
# => evaluates to the set containing 1, 2, and 3o

Hash[:key1, 1, :key2, 2]
# => evaluates to the hash `{key1: 1, key2: 2}`
```

To avoid clobbering any existing `[]` method on these standard library classes,
Sorbet defines classes in the `T::` namespace that mirror the names of classes
in the standard library.

> Note that this mapping is not automatic: Sorbet has special-cased each
> individual class in the table above inside Sorbet (it is not possible to use
> merely prepend `T::` to the name of any class that has been defined as a
> generic type in an [RBI file](rbi.md)).

## "Generic class without type arguments"

For backwards compatibility reasons during Sorbet's original rollout, Sorbet
sometimes allows generic classes defined in the Ruby standard library to appear
in type annotations _without_ being provided type arguments:

```ruby
# typed: true
#        ^^^^ important

T.let([], Array) # no error
```

versus:

```ruby
# typed: strict

T.let([], Array)
#         ^^^^^ error: Generic class without type arguments
```

When this happens, Sorbet defaults all missing type arguments to `T.untyped`.

This exception is made only for classes in the Ruby standard library. For all
other generic classes, Sorbet requires that a generic class is provided all of
its required type arguments, even in `# typed: false` files.

This behavior may change in the future, and we strongly discourage relying on it
intentionally.

## Generics and runtime checks

Recall that Sorbet is not only a static type checker, but also a system for
[validating types at runtime](runtime.md).

However, Sorbet completely erases _generic type arguments_ at runtime. When
Sorbet sees a signature like `T::Array[Integer]`, at runtime it will **only**
check whether an argument has class `Array`, but not whether every element of
that array is also an `Integer` at runtime.

This also means that if the element type of an array has type
[`T.untyped`](untyped.md), Sorbet will not report a static error, nor will
Sorbet report a runtime error.

```ruby
sig {params(xs: T::Array[Integer]).void}
def foo(xs); end

untyped_str_array = T::Array[T.untyped].new('first', 'second')
foo(untyped_str_array)
#   ^^^^^^^^^^^^^^^^^ no static error, AND no runtime error!
```

(Also note that unlike other languages that implement generics via type erasure,
Sorbet does not insert runtime casts that preserve type safety at runtime.)

> **Note**: Sorbet used to take these type arguments into account during runtime
> type-checking, but this turned out to be a common and difficult-to-debug
> source of performance problems, frequently turning a fast, constant-time
> algorithm (e.g., `Hash` lookup) into a linear scan checking all element types.
>
> In order to verify that an array contained the values it claimed it did, the
> Sorbet runtime used to recursively check the type of every member of a
> collection, which would take a long time for arrays or hashes of a
> sufficiently large size. Consequently, this behavior has been removed.

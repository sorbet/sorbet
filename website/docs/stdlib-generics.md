---
id: stdlib-generics
title: Arrays, Hashes, and Generics in the Standard Library
sidebar_label: Arrays & Hashes
---

> TODO: This page is still a fragment. Contributions welcome!

Sorbet supports generic types. The syntax looks likes `MyClass[Elem]`. For user
defined generic classes, it's possible to make this valid Ruby syntax.

```ruby
# typed: true
class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {returns(Elem)}
  attr_reader :x

  sig {params(x: Elem).returns(Elem)}
  attr_writer :x
end

Box[Integer].new.x + Box[String].new.x # error
Box[Integer].new.x -= 10 # good
Box[String].new.x -= 10 # bad Method - does not exist on String
```

However, it's not possible to change the syntax for classes in the Ruby standard
library that should be generic. To make up for this, we use wrappers in the `T`
namespace to represent them:

- `T::Array[Type]`
- `T::Hash[KeyType, ValueType]`
- `T::Set[Type]`
- `T::Enumerable[Type]`
- `T::Enumerator[Type]`
- `T::Range[Type]`

Inside a sig, just writing `Array` or `Hash` will be treated as
`T::Array[T.untyped]` and `T::Hash[T.untyped, T.untyped]` respectively.

```ruby
# typed: true
extend T::Sig

sig {returns(Array)}
def foo
  [1, "heh"]
end

T.reveal_type(foo.first) # Revealed type: T.untyped

sig {returns(T::Array[T.any(Integer, String)])}
def bar
  [1, "heh"]
end

T.reveal_type(bar.first) # Revealed type: T.nilable(T.any(Integer, String))
```

The type parameters for the generic types provided by the standard library are
only used as hints for the static type system, and are ignored entirely by the
runtime type system.

> **Note**: Sorbet used to take these type parameters into account during
> runtime type-checking, but this turned out to be a common and
> difficult-to-debug source of performance problems: In order to verify that an
> array contained the values it claimed it did, the Sorbet runtime used to
> recursively check the type of every member of a collection, which would take a
> long time for arrays or hashes of a sufficiently large size. Consequently,
> this behavior was eventually removed.

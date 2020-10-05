---
id: user-defined-generics
title: User Defined Generics
sidebar_label: User Defined Generics
---

You can define your own generic types, in addition to Sorbet's included
[standard library generics](stdlib-generics.md) like `T::Array`.

This is done by extending `T::Generic` in your class or module, and then 
creating `type_member` constants for each type parameter that your type needs
to have.

Here's an example of creating a generic `Box` type, which has one type parameter
and holds a value of that type:

```ruby
class Box
  extend T::Sig
  extend T::Generic

  # This type has one type member, named "BoxedType"
  BoxedType = type_member

  sig { params(value: BoxedType).void }
  def initialize(value)
    @value = value
  end

  sig { returns(BoxedType) }
  attr_accessor :value
end

# Omitted type members are set to T.untyped
T.reveal_type(Box.new(3)) # Box[T.untyped]

T.reveal_type(Box[Integer].new(3)) # Box[Integer]
T.reveal_type(Box[Integer].new(3).value) # Integer

T.reveal_type(Box[Integer].new("hello")) # error: Expected Integer but found String
```

You can define classes which accept multiple type parameters by using
`type_member` several times, in order:

```ruby
class Pair
  extend T::Sig
  extend T::Generic

  K = type_member
  V = type_member

  sig { params(key: K, value: V).void }
  def initialize(key, value)
    @key = key
    @value = value
  end

  sig { returns(K) }
  attr_accessor :key

  sig { returns(V) }
  attr_accessor :value
end

pair = Pair[String, Integer].new("foo", 2)
T.reveal_type(pair) # Pair[String, Integer]
T.reveal_type(pair.key) # String
T.reveal_type(pair.value) # Integer
```

Using `[]` on a class would normally raise an exception, so extending
`T::Generic` defines this as a method which simply returns your class.

## Re-declaring Type Members

If you inherit from a class, include a module, or extend a module which defines
type members, then the type members must be re-declared in the child type.

There are two ways to re-declare a type member:

- **As a variant type member:** it must still be specified as a type parameter
  when using the type (or left unspecified to use `T.untyped`), and will need to
  be re-declared again in any child types.

- **As a fixed type member:** it will take on a fixed value in this type, and
  cannot be specified with a type parameter. Child types will need to re-declare
  the type member, and this re-declaration must be fixed.

```ruby
# Used like: Base[A, B]
class Base
  extend T::Generic
  A = type_member
  B = type_member
end

# Usage is the same: Subtype[A, B]
class Subtype < Base
  A = type_member
  B = type_member
end

# B has been fixed, so now only A can be specified: PartiallyFixedSubtype[A]
# Any signatures which use B will treat B as String
class PartiallyFixedSubtype < Subtype
  A = type_member
  B = type_member(fixed: String)
end

# Both types are fixed, so FullyFixedSubtype takes no type parameters
# A will be treated as Integer, and B as String
# (Even though the superclass fixed B to String, it must be fixed again)
class FullyFixedSubtype < PartiallyFixedSubtype
  A = type_member(fixed: Integer)
  B = type_member(fixed: String)
end
```

## Variance

Type members on modules and interfaces may be invariant (default), covariant, or
contravariant.

| Variance      | Definition          | Allowed usage in `sig` | Allowed types                |
| ------------- | ------------------- | ---------------------- | ---------------------------- |
| Invariant     | `type_member`       | Any                    | Only exactly as specified    |
| Covariant     | `type_member(:out)` | Return type            | As specified or more derived |
| Contravariant | `type_member(:in)`  | Parameter type         | As specified or less derived |

To demonstrate each variance, we can consider an interface `I` defined with one
type parameter, and a class `A` which includes that interface.

```ruby
module I
  extend T::Generic
  Type = type_member # Variance goes here
end

class A
  extend T::Generic
  include I
  Type = type_member
end

# === Invariant
T.let(A[Object].new, I[BasicObject])  # error
T.let(A[Object].new, I[Object])       # OK
T.let(A[Object].new, I[Integer])      # error

# === Covariant (type_member(:out))
T.let(A[Object].new, I[BasicObject])  # OK - Object is a subtype of BasicObject
T.let(A[Object].new, I[Object])       # OK
T.let(A[Object].new, I[Integer])      # error

# === Contravariant (type_member(:in))
T.let(A[Object].new, I[BasicObject])  # error
T.let(A[Object].new, I[Object])       # OK
T.let(A[Object].new, I[Integer])      # OK - Object is a supertype of Integer
```

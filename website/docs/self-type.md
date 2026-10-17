---
id: self-type
title: T.self_type
---

> **Warning**: This feature is experimental and has known limitations. It may not work as expected or change without notice. See [related issues](https://github.com/sorbet/sorbet/milestone/29).

```ruby
T.self_type
```

This type can be used in return types to indicate that calling this method on will return the same type as the type of the receiver (the receiver is the thing we call a method on i.e., `x` in `x.foo`). For instance, `#dup` returns `T.self_type`. No matter what class you call it on, you will get back the same type.

```ruby
# typed: true

class Parent
  extend T::Sig

  sig { returns(T.self_type) }
  def foo
    self
  end
end

class Child < Parent; end

T.reveal_type(Parent.new.foo) # Revealed type: Parent
T.reveal_type(Child.new.foo) # Revealed type: Child

module Mixin
  extend T::Sig

  sig { returns(T.self_type) }
  def bar
    self
  end
end

class UsesMixin
  extend Mixin
end

T.reveal_type(UsesMixin.bar) # Revealed type: T.class_of(UsesMixin)
```

Note that `T.self_type` declares that the type should be exactly what the type of the receiver of the method is. It is **not** useful for declaring the type of factory methods. For these types of methods, use [`T.attached_class`](attached-class.md) instead:

```ruby
class Parent
  # sig { returns(T.self_type) } # WRONG
  sig { returns(T.attached_class) }
  def self.make
    new
  end
end

class Child < Parent; end

T.reveal_type(Parent.make) # => `Parent`
T.reveal_type(Child.make)  # => `Child`
```

If the above example was declared using `T.self_type`, Sorbet would expect to see the `make` method return a value of type `T.class_of(Parent)`, but `new` returns a value of type `Parent` (an instance of the class, not the class object itself).

# Do not use `T.self_type` for `==`

It's tempting to use `T.self_type` in the definition of an object's `==` method, but this is incorrect.

```ruby
sig { params(other: T.self_type).returns(T::Boolean) }
#            ^^^^^^^^^^^^^^^^^^ ⛔ Incorrect!
def ==(other)
end
```

Ruby equality methods must accept values of an arbitrary class, even if they will return `false` when `self.class != other.class`. As such, equality methods must accept something broad, like `BasicObject` or `T.anything`:

```ruby
sig { params(other: T.anything).returns(T::Boolean) }
def ==(other)
end
```

For more, see [this FAQ entry](faq.md#how-do-i-override--what-signature-should-i-use)

# `T.self_type` is not checked at runtime

`T.self_type` does not get [runtime type checking](runtime.md) like most other types. In this way, it behaves similarly to how [generic types are runtime-erased](generics.md#generics-and-runtime-checks) and thus not runtime checked.

# Limitations

## Methods that return `T.self_type` are not checked precisely

Sorbet incorrectly typechecks methods declared to return `T.self_type`:

```ruby
class Parent
  sig { returns(T.self_type) }
  def returns_same_type
    Parent.new # ‼️ incorrect!
  end
end

class Child < Parent; end

p = Parent.new.returns_same_type
T.reveal_type(p) # => `Parent`

c = Child.new.returns_same_type
T.reveal_type(c) # => `Child`
```

The meaning of `T.self_type` is "whatever the type is of the thing the method was called on," or the method's _receiver_. The method's receiver varies. A method might be defined on `Parent` but called on `Child`, in which case `T.self_type` is equivalent to `Child`.

Sorbet models this relationship at call to methods that return `T.self_type`, but does not check this property inside methods bodies that return `T.self_type`. There is an uncaught bug in the snippet above: Sorbet allows `returns_same_type` to unconditionally return `Parent.new`, which means that calls on subclasses will have an inferred type that does not match what the real class is at runtime.

Methods that return `T.self_type` should use `self` to compute the value that is eventually returned. For example, all of these are valid ways to use `self` to return something of type `T.self_type`:

```ruby
  self # ✅
  self.class.new # ✅
  self.clone # ✅
  # ...
```

Sorbet does not yet check this property but will one day. Please follow or upvote [this issue](https://github.com/sorbet/sorbet/issues/775).

## Only top-level

Sorbet only supports `T.self_type` at the top-level of a type, e.g. not nested inside a generic class type:

```ruby
class Generic < Parent
  extend T::Generic
  TM = type_member

  sig { returns(Generic[T.self_type]) } # error: Only top-level `T.self_type` is supported
  def bad
    Generic[T.untyped].new
  end
end
```

Note that "top-level" in this context only applies to inside the type arguments applied to generic class types—`T.self_type` can already be nested inside types like `T.any` and `T.all`.

This limitation is likely to be lifted eventually, so please let us know whether that's important.

[`T.proc` types](procs.md) are exempt from this restriction, even though they're generic class types in disguise: `T.self_type` can appear in a `T.proc`'s parameter and return types (as well as in `T.proc.bind`). This is what lets Sorbet give methods like `then`, `yield_self`, and `tap` their natural types:

```ruby
sig do
  type_parameters(:U)
    .params(blk: T.proc.params(arg0: T.self_type).returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
end
def my_then(&blk)
  yield self
end
```

The usual [variance rules](#no-uses-in-parameters-types) still apply, so `T.self_type` is only allowed where it ends up being used covariantly—like the parameters of a `T.proc` that is itself a method parameter, as above.

## No uses in parameters' types

Using `T.self_type` in a method's parameter is rejected:

```ruby
class A
  sig { params(other: T.self_type).void }
  def foo(other)
    other
  end
end
```

One exception is if the method is private:

```ruby
class Parent
  sig { params(other: T.self_type).void }
  private_class_method def inherited(other)
    other
  end
end
```

If it helps, think of `T.self_type` as a [type_member](generics.md#type_member--type_template) that is [covariant](generics.md#covariance-out) and [upper bounded](generics.md#bounds-on-type_members-and-type_templates-fixed-upper-lower) by itself (i.e., a recursively-defined type). Just as covariant type members are not allowed in input positions, neither is `T.self_type`, unless [the method is private](generics.md#variance-positions-and-private).

This is a rule about positions, not about the word `params`. Two contravariant positions cancel out, so `T.self_type` **is** allowed in the parameters of a `T.proc` that is itself a method parameter (which is how methods like `tap` are typed):

```ruby
class A
  extend T::Sig

  # ✅ `T.self_type` is used covariantly overall
  sig { params(blk: T.proc.params(x: T.self_type).void).void }
  def each_self(&blk)
    yield self
  end

  # ❌ `T.self_type` is used contravariantly overall
  sig { returns(T.proc.params(x: T.self_type).void) }
  def bad
    ->(x) {}
  end
end
```

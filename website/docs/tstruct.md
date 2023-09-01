---
id: tstruct
title: Typed Structs via T::Struct
sidebar_label: T::Struct
---

Sorbet includes a way to define typed structs. They behave similarly to the
[`Struct`] class built into Ruby, but work better with static and runtime type
checking.

[`struct`]: https://docs.ruby-lang.org/en/master/Struct.html

Here's a quick example:

```ruby
class MonetaryAmount < T::Struct
  # (1) Define mutable struct properties with the `prop` DSL
  # (like a typed version of `attr_accessor`)
  prop :amount, Integer

  # (2) Define constant struct properties with the `const` DSL
  # (like a typed version of `attr_reader`)
  const :currency, String
end

# (3) T::Struct constructors always take arguments via keywords
monetary = MonetaryAmount.new(amount: 1000, currency: 'USD')

# (4) Access the values using getters and setters
p(monetary.amount) # => 1000
monetary.amount = 2100

# (5) `const` properties cannot be updated
monetary.currency = 'GBP'
#       ^^^^^^^^^^^ undefined method `currency=`

# (6) Everything is type checked, unlike Ruby's `Struct` class
MonetaryAmount.new(amount: 1000)
# ^ error: Missing required keyword argument `currency`
MonetaryAmount.new(amount: 'not an int', currency: 'USD')
# ^ error: Expected `Integer` but found `String`
monetary.amount + 'not an int'
# ^ error: Expects an `Integer`, not `String`
monetary.amount = 'not an int'
# ^ error: Expected `Integer` but found `String`
```

[→ View example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20MonetaryAmount%20%3C%20T%3A%3AStruct%0A%20%20%23%20%281%29%20Define%20mutable%20struct%20properties%20with%20the%20%60prop%60%20DSL%0A%20%20%23%20Like%20a%20typed%20version%20of%20%60attr_accessor%60%0A%20%20prop%20%3Aamount%2C%20Integer%0A%0A%20%20%23%20%282%29%20Define%20constant%20struct%20properties%20with%20the%20%60const%60%20DSL%0A%20%20%23%20Like%20a%20typed%20version%20of%20%60attr_reader%60%0A%20%20const%20%3Acurrency%2C%20String%0Aend%0A%0A%23%20%283%29%20T%3A%3AStruct%20constructors%20always%20take%20arguments%20via%20keywords%0Amonetary%20%3D%20MonetaryAmount.new%28amount%3A%201000%2C%20currency%3A%20'USD'%29%0A%0A%23%20%284%29%20Access%20the%20values%20using%20getter%20and%20setters%0Ap%28monetary.amount%29%20%23%20%3D%3E%201000%0Amonetary.amount%20%3D%202100%0A%0A%23%20%285%29%20Attempting%20to%20set%20a%20%60const%60%20property%20results%20in%20a%20%60NoMethodError%60%0Amonetary.currency%20%3D%20'GBP'%0A%23%20%20%20%20%20%20%20%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%5E%20undefined%20method%20%60currency%3D%60%0A%0A%23%20%286%29%20Everything%20is%20type%20checked%2C%20unlike%20Ruby's%20%60Struct%60%20class%0AMonetaryAmount.new%28amount%3A%201000%29%20%23%20error%0AMonetaryAmount.new%28amount%3A%20'not%20an%20int'%29%20%23%20error%0Amonetary.amount%20%2B%20'not%20an%20int'%20%23%20error%0Amonetary.amount%20%3D%20'not%20an%20int'%20%23%20error)

## Optional properties: `T.nilable`, `default:`, and `factory:`

By default, all `T::Struct` properties are required on initialization. There are
three ways to mark a property as optional:

1.  Provide a `default: ...` keyword argument to the `prop` or `const`.

    The provided value will be used if that property is omitted at
    initialization time.

2.  Provide a proc or lambda via the `factory: ...` keyword argument on a `prop`
    or `const`.

    This is similar to `default:`, but the argument will be called (with no
    arguments) to produce a default value when needed.

3.  Declare the prop's type as a `T.nilable(...)` type.

    Not only will this allow the prop's value to include `nil`, but it also
    implies `default: nil` if no explicit `default:` or `factory:` value is
    provided.

```ruby
class OptionalExample < T::Struct
  # All these props are optional
  prop :uses_default, String, default: ''
  prop :created, Float, factory: ->() { Time.now.to_f }
  prop :nilable, T.nilable(Integer)
end

x = OptionalExample.new
x.uses_default # => ''
x.created      # => 1666483572.897899
x.nilable      # => nil

y = OptionalExample.new
x.uses_default # => ''
x.created      # => 1666483576.475571
x.nilable      # => nil
```

### Default values and references

To avoid having a default value be shared and mutated by **all** instances of a
`T::Struct`, certain built-in types are deeply cloned at initialization time.
Other types that are not built into Ruby have their `.clone` method called.

Before we get ahead of ourselves, consider this code:

```ruby
class Example < T::Struct
  # The `[]` default is cloned on initialization,
  # so it is not shared by multiple instances.
  prop :vals, T::Array[Integer], default: []
end

ex1 = Example.new
ex2 = Example.new
ex1.vals << 'elem'
p(ex2.vals)
```

It would be surprising if `p(ex2.vals)` printed `['elem']` in this example—it
would mean that the default of `[]` was shared by reference across all `Example`
instances, so that updating one instance's `vals` property simultaneously
affected all of them.

To fix this, `T::Struct` takes measures to clone objects, so that they are not
shared:

- `true`, `false`, `nil`, any `Symbol`, any `Numeric`, and `T::Enum` values are
  either value objects (not reference objects) or are known to be immutable, and
  so are not cloned when being used as a default.
- `String` instances that are frozen (according to `frozen?`) are not cloned,
  for performance. All other `String`s have `.clone` called on them before being
  used as a default value.
- `Array` and `Hash` default values are deeply cloned (i.e., Sorbet recursively
  calls `.clone` not only on the `Array` or `Hash` itself, but also on all their
  elements).
- All other default values are simply cloned by calling `.clone` on the provided
  default.

These rules prevent the most common misuses of accidentally mutating default
values via references, but it is still possible to construct cases where the
above rules are not strong enough. In such cases, use `factory:` to compute the
default value in whatever way necessary. The value produced by `factory:` is
used verbatim. (This means that `factory:` can be used when reference sharing
across default values is actually the _desired_ outcome.)

## Structs and inheritance

Sorbet does not allow inheriting from a class which inherits from `T::Struct`.

```ruby
class S < T::Struct
  prop :foo, Integer
end

class Bad < S; end # error
```

Sorbet imposes this limitation somewhat artificially, for performance. Sorbet
generates a static signature for the `initialize` method of a `T::Struct`
subclass. In order to do so, it needs to know all `prop`'s defined on the class.
For performance in large codebases, Sorbet requires that it is possible to know
which methods are defined on a `T::Struct` class purely based on syntax—Sorbet
does not allow discovering a `T::Struct`'s properties via ancestor information,
like the class's superclass or mixins.

One common situation where inheritance may be desired is when a parent struct
declares some common props, and child structs declare their own props:

```ruby
class Parent < T::Struct
  prop :foo, Integer
end

class ChildOne < Parent # error
  prop :bar, String
end

class ChildTwo < Parent # error
  prop :quz, Symbol
end
```

This code can be restructured to use composition instead of inheritance:

```ruby
class Common < T::Struct
  prop :foo, Integer
end

class ChildOne < T::Struct
  prop :common, Common
  prop :bar, String
end

class ChildTwo < T::Struct
  prop :common, Common
  prop :quz, Symbol
end
```

Another option is to define a common interface, and repeat the props in each
child class:

```ruby
module Common
  extend T::Helpers
  extend T::Sig
  interface!
  sig {abstract.returns(Integer)}
  def foo; end
  sig {abstract.params(foo: Integer).returns(Integer)}
  def foo=(foo); end
end

class ChildOne < T::Struct
  include Common
  prop :foo, Integer
  prop :bar, String
end

class ChildTwo < T::Struct
  include Common
  prop :foo, Integer
  prop :quz, Symbol
end
```

If the code absolutely must use inheritance and cannot use composition, either:

- Avoid using `T::Struct`, and instead define a normal class, with things like
  `attr_reader` and an explicit `initialize` method.

- Change the superclass from `T::Struct` to `T::InexactStruct`. This causes
  Sorbet to no longer statically check the types of any arguments passed to the
  `initialize` method on the subclass, but does allow defining `T::Struct`
  hierarchies. This should only be used as a last resort.

<br>

---

<br>

## Legacy code and historical context

The `prop` DSL used by `T::Struct` predates Sorbet by about 5 years. It was
originally conceived at Stripe in early 2013 to form the basis for Stripe's
internal [object-document mapper][odm] (ODM). By the time Stripe began internal
development on Sorbet in late 2017, Stripe's ODM was by far the most commonly
used internal abstraction for associating types with methods. At a time when it
was not clear that the as-yet-unnamed Ruby type checker project would succeed or
not, we were eager to build on existing abstractions to bootstrap early type
coverage.

[odm]:
  https://en.wikipedia.org/wiki/Object%E2%80%93relational_mapping#Object-oriented_databases

A decision was made to factor the code for the `prop` DSL into a standalone
library, to allow using it independently of the database-specific code in
Stripe's ODM library. From this effort, `T::Struct` was born. A `T::Struct` is
essentially a Stripe [database model] class without the database.

[database model]: https://en.wikipedia.org/wiki/Database_model

Unfortunately, this process left warts in the publicly-accessible `T::Struct`
APIs that persist today. Certain parts of the `prop` DSL only make sense when
used alongside Stripe-internal abstractions. The DSL also contains things that
are technically publicly accessible that were never meant to be. This legacy
makes it hard to evolve and improve the `T::Struct` APIs without breaking
existing code.

The remainder of this documentation is presented for completeness. Use the APIs
below at your own discretion. Our goal here is simply to outline the potential
pitfalls that arise when using them.

## `serialize` and `from_hash`: Converting `T::Struct` to and from `Hash`

It's possible to convert a `T::Struct` instance to and from `Hash` instances:

```ruby
class A < T::Struct
  prop :foo, Integer
end

# (1) `serialize` converts from `T::Struct` to `Hash`
serialized = A.new(foo: 42).serialize
p(serialized) # => {"foo"=>42}

# (1) `from_hash` converts from `Hash` to `T::Struct`
deserialized = A.from_hash(serialized)
p(deserialized) # => <A foo=42>
```

**However, `serialize` and especially `from_hash` are particularly fraught**
(see the "gotchas" sections below). It's likely better to do manual conversion
to and from `Hash` values:

```ruby
# (1) Convert to hashes directly
class A < T::Struct
  prop :foo, Integer
end

a = A.new(foo: 12)
as_hash = {
  foo: a.foo
}

# (2) Use keyword splat arguments with `new` to convert from a `Hash`
A.new(**as_hash)
```

### Custom serializations with `name:`

The `name:` option on the `prop` DSL controls the field name that will be used
when converting to and from `Hash` values:

```ruby
class A < T::Struct
  # (1) The name `fooBar` will be used when converting to/from `Hash` values
  prop :foo_bar, Integer, name: "fooBar"
end

serialized = A.new(foo_bar: 42).serialize
p(serialized) # => {"fooBar"=>42}

deserialized = A.from_hash(serialized)
p(deserialized) # <A foo_bar=42>
```

### `serialize` gotchas

As mentioned in the [previous section][legacy], the `serialize` behavior was
inherited from Stripe's internal ODM library, and thus has some warts to be
aware of:

- The `Hash` has `String`-valued keys, unlike Ruby's `Struct#to_h` method, which
  produces `Symbol`-valued keys. Even custom names provided with `name:` must be
  `String`s.

- `nil` properties are omitted from the resulting `Hash`.

- Nested `T::Struct` and `T::Enum` values are also serialized:

  ```ruby
  class Nested < T::Struct
    prop :bar, Integer
  end

  class XorY < T::Enum
    enums do
      X = new
      Y = new
    end
  end

  class Top < T::Struct
    prop :nested, Nested
    prop :x_or_y, XorY
  end

  p(Top.new(nested: Nested.new(bar: 42), x_or_y: XorY::X).serialize)
  # => {"nested"=>{"bar"=>42}, "x_or_y"=>"x"}
  ```

- **However**, [union-typed](union-types.md) properties containing `T::Struct`
  instances are **not** serialized:

  ```ruby
  class Foo < T::Struct
    prop :foo, Integer
  end
  class Bar < T::Struct
    prop :bar, String
  end

  class Top < T::Struct
    prop :foo_or_bar, T.any(Foo, Bar)
  end

  foo_top = Top.new(foo_or_bar: Foo.new(foo: 12))

  foo_serialized = foo_top.serialize
  p(foo_serialized) # => {"foo_or_bar"=><Foo foo=12>}
  ```

- Same with [generic-typed](generics.md) properties containing `T::Struct`
  instances: these are also not serialized.

### `from_hash` gotchas

As mentioned in the [previous section][legacy], the `deserialize` behavior was
inherited from Stripe's internal ODM library, and thus has some warts to be
aware of.

- The `Hash` given to `from_hash` must have `String`-valued keys, like the
  result of calling `serialize`.

- The `from_hash` method does not do the same static nor runtime type checking
  that the `T::Struct`'s `new` method would do:

  - There are no static type checks.
  - Required properties missing in the `Hash` **do** raise exceptions at
    runtime.
  - Extra or unknown properties present in the `Hash` do not raise exceptions at
    runtime unless the optional `strict` argument to `from_hash` is passed (or
    the method is called via the `from_hash!` wrapper).
  - The types provided via the `Hash` are **not** checked at runtime.

- Because [union-typed](union-types.md) properties containing `T::Struct`
  instances are not serialized, they must also not be still serialized when
  given to `from_hash`:

  ```ruby
  class Foo < T::Struct
    prop :foo, Integer
  end
  class Bar < T::Struct
    prop :bar, String
  end

  class Top < T::Struct
    prop :foo_or_bar, T.any(Foo, Bar)
  end

  foo = Foo.new(foo: 12)
  p(Top.from_hash({"foo_or_bar" => foo}))
  # => <Top foo_or_bar=<Foo foo=12>>
  p(Top.from_hash({"foo_or_bar" => foo.serialize}))
  # => <Top foo_or_bar={"foo"=>12}>
  ```

  And since there are no runtime type checks, the serialized hash value is
  directly set to the `foo_or_bar` field.

## Structural vs reference equality

By default, `T::Struct` values compare using reference equality ("Are these two
instances literally the same object in memory?""), while classes created with
Ruby's `Struct` class compare using structural equality ("Are these two
possibly-different objects both instances of the same class, containing
pairwise-equal fields?").

While it would be nice if `T::Struct` had been built from the beginning with
structural equality, it wasn't, and now quite a lot of code in the wild depends
on this.

For those cases where structural equality is preferred, we recommend defining a
custom module that can be included into a `T::Struct` to override the equality
methods, providing structural equality.

## Immutable property updates using `with`

Properties defined with `const` do not have setter methods, making it impossible
to update these properties after construction. A common pattern when working
with such classes is to "immutably update" the instance by creating a copy of an
object with identical fields except with a different value for the one `const`
property.

This is built into `T::Struct`, but has some limitations:

```ruby
class A < T::Struct
  const :foo, Integer
  const :another_required, Integer
end

a1 = A.new(foo: 1, another_required: 42)
p(a1) # => <A foo=1 another_required=42>

# The `with` method
a2 = a1.with(foo: 2)
p(a2) # => <A foo=2 another_required=42>
```

Added in haste, the implementation of `with` uses `from_hash` to merge the new
and old properties and create the new instance. This means it suffers from
exactly the same gotchas mentioned in the
[`from_hash` gotchas](#from_hash-gotchas) section above.

## Legacy and Stripe-specific options

There are a number of other legacy or Stripe-internal options in the `prop` DSL.
Those include `dont_store`, `enum`, `foreign`, `ifunset`, `immutable`,
`raise_on_nil_write`, `redaction`, and `sensitivity`. Stripe employees can
reference [these docs](http://go/chalk-odm-docs) to learn more.

Other users of `sorbet-runtime` are not encouraged to use these options.

[legacy]: #legacy-code-and-historical-context

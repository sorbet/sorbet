---
id: tenum
title: Typed Enums via T::Enum
sidebar_label: T::Enum
---

Enumerations allow for type-safe declarations of a fixed set of values. "Type
safe" means that the values in this set are the only values that belong to this
type. Here's an example of how to define a typed enum with Sorbet:

```ruby
# (1) New enumerations are defined by creating a subclass of T::Enum
class Suit < T::Enum
  # (2) Enum values are declared within an `enums do` block
  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end
end
```

Note how each enum value is created by calling `new`: each enum value is an
instance of the enumeration class itself. This means that
`Suit::Spades.is_a?(Suit)`, and the same for all the other enum values. This
guarantees that one enum value cannot be used where some other type is expected,
and vice versa.

This also means that once an enum has been defined as a subclass of `T::Enum`,
it behaves like any other [Class Type](class-types.md) and can be used in method
signatures, type aliases, `T.let` annotations, and any other place a class type
can be used:

```ruby
sig {returns(Suit)}
def random_suit
  Suit.values.sample
end
```

## Exhaustiveness

Sorbet knows about the values in an enumeration statically, and so it can use
[exhaustiveness checking](exhaustiveness.md) to check whether all enum values
have been considered. The easiest way is to use a `case` statement:

```ruby
sig {params(suit: Suit).void}
def describe_suit_color(suit)
  case suit
  when Suit::Spades   then puts "Spades are black!"
  when Suit::Hearts   then puts "Hearts are red!"
  when Suit::Clubs    then puts "Clubs are black!"
  when Suit::Diamonds then puts "Diamonds are red!"
  else T.absurd(suit)
  end
end
```

Because of the call to `T.absurd`, if any of the individual suits had not been
handled, Sorbet would report an error statically that one of the cases was
missing. For more information on how exhaustiveness checking works, see
[Exhaustiveness Checking](exhaustiveness.md).

## Converting enums to other types

Enumerations do not implicitly convert to any other type. Instead, all
conversion must be done explicitly. One particularly convenient way to implement
these conversion functions is to define instance methods on the enum class
itself:

```ruby
class Suit < T::Enum
  enums do
    # ...
  end

  sig {returns(Integer)}
  def rank
    # (1) Case on self (because this is an instance method)
    case self
    when Spades then 1
    when Hearts then 2
    when Clubs then 3
    when Diamonds then 4
    else
      # (2) Exhaustiveness still works when casing on `self`
      T.absurd(self)
    end
  end
end
```

A particularly common case to convert an enum to a String. Because this is so
common, this conversion has been built in (it still must be explicitly called):

```ruby
Suit::Spades.serialize # => 'spades'
Suit::Hearts.serialize # => 'hearts'
# ...
```

Again: this conversion must be done explicitly. When attempting to implicitly
convert an enum value to a string, you'll get a non-human-friendly
representation of the enum:

```ruby
suit = Suit::Spades
puts "Got suit: #{suit}"
# =>  Got suit: #<Suit::Spades>
```

The default value used for serializing an enum is the name of the enum, all
lowercase. To specify an alternative serialized value, pass an argument to
`new`:

```ruby
class Suit < T::Enum
  enums do
    Spades = new('SPADES')
    Hearts = new('HEARTS')
    Clubs = new('CLUBS')
    Diamonds = new('DIAMONDS')
  end
end

Suit::Diamonds.serialize # => 'DIAMONDS'
```

Each serialized value must be unique compared to all other serialized values for
this enum.

Another common conversion is to take the serialized value and deserialize it
back to the original enum value. This is also built into `T::Enum`:

```ruby
serialized = Suit::Spades.serialize
suit = Suit.deserialize(serialized)

puts suit
# => #<Suit::Spades>
```

When the value being deserialized doesn't exist, a `KeyError` exception is
raised:

```ruby
Suit.deserialize('bad value')
# => KeyError: Enum Suit key not found: "bad value"
```

If this is not the behavior you want, you can use `try_deserialize` which
returns `nil` when the value doesn't deserialize to anything:

```ruby
Suit.try_deserialize('bad value')
# => nil
```

You can also ask whether a specific serialized value exists for an enum:

```ruby
Suit.has_serialized?(Suit::Spades.serialize)
# => true

Suit.has_serialized?('bad value')
# => false
```

<!-- TODO(jez) Using enum *values* as type annotations / in unions -->
<!-- TODO(jez) ^ Limitation: can't be used in type aliases... -->

## Listing the values of an enum

Sometimes it is useful to enumerate all the values of an enum:

```ruby
Suit.values
# => [#<Suit::Spades>, #<Suit::Heart>, #<Suit::Clubs>, #<Suit::Diamonds>]
```

## Attaching metadata to an enum

It can be tempting to "attach metadata" to each enum value by overriding the
constructor for a `T::Enum` subclass such that it accepts more information and
stores it on an instance variable.

This is **strongly discouraged**. It's likely that Sorbet will enforced this
discouragement with a future change.

Concretely, consider some code like this that is discouraged:

<a href="https://sorbet.run/#%23%20typed%3A%20strict%0Aclass%20Suit%20%3C%20T%3A%3AEnum%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_reader%20%3Arank%0A%0A%20%20sig%20%7Bparams(serialized_val%3A%20String%2C%20rank%3A%20Integer).void%7D%0A%20%20def%20initialize(serialized_val%2C%20rank)%0A%20%20%20%20super(serialized_val)%0A%20%20%20%20%40rank%20%3D%20T.let(rank%2C%20Integer)%0A%20%20end%0A%0A%20%20enums%20do%0A%20%20%20%20Spades%20%3D%20new('spades'%2C%201)%0A%20%20%20%20Hearts%20%3D%20new('hearts'%2C%202)%0A%20%20%20%20Clubs%20%3D%20new('clubs'%2C%203)%0A%20%20%20%20Diamonds%20%3D%20new('diamonds'%2C%204)%0A%20%20end%0Aend%0A">→
View on sorbet.run</a>

This code is discouraged because it...

- overrides the `T::Enum` constructor, making it brittle to potential future
  changes in the `T::Enum` API.
- stores state on each enum value. Enum values are singleton instances, meaning
  that if someone accidentally mutates this state, it's observed globally
  throughout an entire program.

Rather than thinking of enums as data containers, instead think of them as dumb
immutable values. A more idiomatic way to express the code above looks similar
to the example given in the section [Converting enums to other
types](#converting-enums-to-other types) above:

```ruby
# typed: strict
class Suit < T::Enum
  extend T::Sig

  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end

  sig {returns(Integer)}
  def rank
    case self
    when Spades then 1
    when Hearts then 2
    when Clubs then 3
    when Diamonds then 4
    else T.absurd(self)
    end
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20strict%0Aclass%20Suit%20%3C%20T%3A%3AEnum%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20enums%20do%0A%20%20%20%20Spades%20%3D%20new%0A%20%20%20%20Hearts%20%3D%20new%0A%20%20%20%20Clubs%20%3D%20new%0A%20%20%20%20Diamonds%20%3D%20new%0A%20%20end%0A%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20def%20rank%0A%20%20%20%20case%20self%0A%20%20%20%20when%20Spades%20then%201%0A%20%20%20%20when%20Hearts%20then%202%0A%20%20%20%20when%20Clubs%20then%203%0A%20%20%20%20when%20Diamonds%20then%204%0A%20%20%20%20else%20T.absurd(self)%0A%20%20%20%20end%0A%20%20end%0Aend">→
View on sorbet.run</a>

This example uses [exhaustiveness](exhaustiveness.md) on the enum to associate a
rank with each suit. It does this without needing to override anything built
into `T::Enum`, and without mutating state.

> If you need exhaustiveness over a set of cases which do carry data, see
> [Approximating algebraic data types](sealed.md#approximating-algebraic-data-types).

## What's next?

- [Union types](union-types.md)

  Enums are great for defining simple sets of related constants. When the values
  are not simple constants (for example, "any instance of these two classes"),
  union types provide a more powerful mechanism for organizing code.

- [Sealed Classes and Modules](sealed.md)

  While union types provide an ad hoc mechanism to group related types, sealed
  classes and modules provide a way to establish this grouping at these types'
  definitions.

- [Exhaustiveness Checking](exhaustiveness.md)

  For union types, sealed classes, and enums, Sorbet has powerful exhaustiveness
  checking that can statically catch when certain cases have not been handled.

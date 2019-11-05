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

You can also ask whether a specific serialized value exists for an enum:

```ruby
Suit.has_serialized?(Suit::Spades.serialize)
# => true

Suit.has_serialized?('bad value')
# => false
```

<!-- TODO(jez) Add a version of deserialize that returns T.nilable? -->
<!-- TODO(jez) Using enum *values* as type annotations / in unions -->
<!-- TODO(jez) ^ Limitation: can't be used in type aliases... -->

## Listing the values of an enum

Sometimes it is useful to enumerate all the values of an enum:

```ruby
Suit.values
# => [#<Suit::Spades>, #<Suit::Heart>, #<Suit::Clubs>, #<Suit::Diamonds>]
```

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

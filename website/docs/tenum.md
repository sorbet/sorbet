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
  T.cast(Suit.values.sample, Suit)
end
```

> The `T.cast` is necessary because of how
> [Sorbet behaves with `Array#sample`](faq.md#sigs-are-vague-for-stdlib-methods-that-accept-keyword-arguments--have-multiple-return-types).

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

## Enum values in types

We might want a type for "only red suits" or "only black suits." `T::Enum`
allows using individual enum values like `Suit::Hearts` or `Suit::Diamonds` as
types (in addition to the name of the enum itself, like `Suit`):

```ruby
# (1) Hearts and Diamonds are enum values and also types
sig {params(red_suit: T.any(Suit::Hearts, Suit::Diamonds)).void}
def handle_red_suit(red_suit)
  case red_suit
  when Suit::Hearts then "..."
  when Suit::Diamonds then "..."
  # (2) exhaustive, even with only two cases handled
  else T.absurd(red_suit)
  end
end

handle_red_suit(Suit::Hearts) # ok
handle_red_suit(Suit::Spades) # type error
```

We can also define [type aliases](type-aliases.md) of various groups of enum
values:

```ruby
# Two type aliases for two different enum subsets
RedSuit = T.type_alias {T.any(Suit::Hearts, Suit::Diamonds)}
BlackSuit = T.type_alias {T.any(Suit::Spades, Suit::Clubs)}

sig {params(black_suit: BlackSuit).void}
def handle_black_suit(black_suit)
  # ...
end
```

Using enum values in types is useful for modeling enums where there is a
frequently-used subset of enum values.

<a href="https://sorbet.run/#%23%20typed%3A%20strict%0Aextend%20T%3A%3ASig%0A%0A%23%20(1)%20New%20enumerations%20are%20defined%20by%20creating%20a%20subclass%20of%20T%3A%3AEnum%0Aclass%20Suit%20%3C%20T%3A%3AEnum%0A%20%20%23%20(2)%20Enum%20values%20are%20declared%20within%20an%20%60enums%20do%60%20block%0A%20%20enums%20do%0A%20%20%20%20Spades%20%3D%20new%0A%20%20%20%20Hearts%20%3D%20new%0A%20%20%20%20Clubs%20%3D%20new%0A%20%20%20%20Diamonds%20%3D%20new%0A%20%20end%0Aend%0A%0Asig%20%7Bparams(red_suit%3A%20T.any(Suit%3A%3AHearts%2C%20Suit%3A%3ADiamonds)).void%7D%0Adef%20handle_red_suit(red_suit)%0A%20%20case%20red_suit%0A%20%20when%20Suit%3A%3AHearts%20then%20%22...%22%0A%20%20when%20Suit%3A%3ADiamonds%20then%20%22...%22%0A%20%20%23%20exhaustive%2C%20even%20with%20only%20two%20cases%20handled%0A%20%20else%20T.absurd(red_suit)%0A%20%20end%0Aend%0A%0Ahandle_red_suit(Suit%3A%3AHearts)%20%23%20ok%0Ahandle_red_suit(Suit%3A%3ASpades)%20%23%20type%20error%0A%0ARedSuit%20%3D%20T.type_alias%20%7BT.any(Suit%3A%3AHearts%2C%20Suit%3A%3ADiamonds)%7D%0ABlackSuit%20%3D%20T.type_alias%20%7BT.any(Suit%3A%3ASpades%2C%20Suit%3A%3AClubs)%7D%0A%0Asig%20%7Bparams(black_suit%3A%20BlackSuit).void%7D%0Adef%20handle_black_suit(black_suit)%0A%20%20case%20black_suit%0A%20%20when%20Suit%3A%3ASpades%20then%20%22...%22%0A%20%20when%20Suit%3A%3AClubs%20then%20%22...%22%0A%20%20%23%20exhaustive%2C%20even%20with%20only%20two%20cases%20handled%0A%20%20else%20T.absurd(black_suit)%0A%20%20end%0Aend%0A">→
View full example on sorbet.run</a>

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

A particularly common case is to convert an enum to a String. Because this is so
common, this conversion method is built in:

```ruby
Suit::Spades.serialize # => 'spades'
Suit::Hearts.serialize # => 'hearts'
# ...
```

Again: this conversion to a string must still be done explicitly. When
attempting to implicitly convert an enum value to a string, you'll get a
non-human-friendly representation of the enum:

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
this enum. The argument to `new` currently accepts `T.untyped`, meaning you can
pass any value to `new` (including things like `Symbol`s or `Integer`s). A
future change to Sorbet may restrict this; we strongly recommend that you pass
`String` values as the explicit serialization values.

## Converting from other types to enums

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

This is **strongly discouraged**. It's likely that Sorbet will enforce this
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
to the example given in the section
[Converting enums to other types](#converting-enums-to-other-types) above:

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

## Defining one enum as a subset of another enum

> This section has been superseded by the
> [Enum values in types](#enum-values-in-types) section above. This section is
> older, and describes workarounds relevant before that section above existed.
> We include this section here mostly for inspiration (the ideas in this section
> are not discouraged, just verbose).

In addition to using [enum values in types](#enum-values-in-types) and type
aliases of enum values, there are two other ways to define one enum as a subset
of another:

1.  By using a [sealed module](sealed.md)
2.  By explicitly converting between multiple enums

Let's elaborate on those two one at a time.

All the examples below will be for days of the week. There are 7 days total, but
there are two clear groups: weekdays and weekends, and sometimes it makes sense
to have the type system enforce that a value can **only** be a weekday enum
value or **only** a weekend enum value.

### By using a sealed module

[Sealed modules](sealed.md) are a way to limit where a module is allowed to be
included. See [the docs](sealed.md) if you'd like to learn more, but here's how
they can be used together with `T::Enum`:

```ruby
# (1) Define an interface / module
module DayOfWeek
  extend T::Helpers
  sealed!
end

class Weekday < T::Enum
  # (2) include DayOfWeek when defining the Weekday enum
  include DayOfWeek

  enums do
    Monday = new
    Tuesday = new
    Wednesday = new
    Thursday = new
    Friday = new
  end
end

class Weekend < T::Enum
  # (3) ditto
  include DayOfWeek

  enums do
    Saturday = new
    Sunday = new
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20strict%0Aclass%20Module%0A%20%20include%20T%3A%3ASig%0Aend%0A%0Amodule%20DayOfWeek%0A%20%20extend%20T%3A%3AHelpers%0A%20%20sealed!%0Aend%0A%0Aclass%20Weekday%20%3C%20T%3A%3AEnum%0A%20%20include%20DayOfWeek%0A%0A%20%20enums%20do%0A%20%20%20%20Monday%20%3D%20new%0A%20%20%20%20Tuesday%20%3D%20new%0A%20%20%20%20Wednesday%20%3D%20new%0A%20%20%20%20Thursday%20%3D%20new%0A%20%20%20%20Friday%20%3D%20new%0A%20%20end%0Aend%0A%0Aclass%20Weekend%20%3C%20T%3A%3AEnum%0A%20%20include%20DayOfWeek%0A%0A%20%20enums%20do%0A%20%20%20%20Saturday%20%3D%20new%0A%20%20%20%20Sunday%20%3D%20new%0A%20%20end%0Aend%0A%0Asig%20%7Bparams(day%3A%20DayOfWeek).void%7D%0Adef%20foo(day)%0A%20%20case%20day%0A%20%20when%20Weekday%3A%3AMonday%20then%20nil%0A%20%20when%20Weekday%3A%3ATuesday%20then%20nil%0A%20%20when%20Weekday%3A%3AWednesday%20then%20nil%0A%20%20when%20Weekday%3A%3AThursday%20then%20nil%0A%20%20when%20Weekday%3A%3AFriday%20then%20nil%0A%0A%20%20when%20Weekend%3A%3ASaturday%20then%20nil%0A%20%20%23when%20Weekend%3A%3ASunday%20then%20nil%0A%20%20else%20T.absurd(day)%0A%20%20end%0Aend%0A">→
view full example on sorbet.run</a>

Now we can use the type `DayOfWeek` for "any day of the week" or the types
`Weekday` & `Weekend` in places where only one specific enum is allowed.

There are a couple limitations with this approach:

1.  Sorbet doesn't allow calling methods on `T::Enum` when we have a value of
    type `DayOfWeek`. Since it's an interface, only the methods defined that
    interface can be called (so for example `day_of_week.serialize` doesn't type
    check).

    One way to get around this is to declare [abstract methods](abstract.md) for
    all of the `T::Enum` methods that we'd like to be able to call (`serialize`,
    for example).

2.  It's not the case that `T.class_of(DayOfWeek)` is a valid
    `T.class_of(T::Enum)`. This means that we can't pass `DayOfWeek` (the class
    object) to a method that calls `enum_class.values` on whatever enum class it
    was given to list the valid values of an enum.

The second approach addresses these two issues, at the cost of some verbosity.

### By explicitly converting between multiple enums

The second approach is to define multiple enums, each of which overlap values
with the other enums, and to define explicit conversion functions between the
enums:

<a href="https://sorbet.run/#%23%20typed%3A%20strict%0Aclass%20Module%0A%20%20include%20T%3A%3ASig%0Aend%0A%0Aclass%20DayOfWeek%20%3C%20T%3A%3AEnum%0A%20%20enums%20do%0A%20%20%20%20Monday%20%3D%20new%0A%20%20%20%20Tuesday%20%3D%20new%0A%20%20%20%20Wednesday%20%3D%20new%0A%20%20%20%20Thursday%20%3D%20new%0A%20%20%20%20Friday%20%3D%20new%0A%0A%20%20%20%20Saturday%20%3D%20new%0A%20%20%20%20Sunday%20%3D%20new%0A%20%20end%0A%0A%20%20sig%20%7Breturns(T.nilable(Weekday))%7D%0A%20%20def%20to_weekday%0A%20%20%20%20case%20self%0A%20%20%20%20when%20Monday%20then%20Weekday%3A%3AMonday%0A%20%20%20%20when%20Tuesday%20then%20Weekday%3A%3ATuesday%0A%20%20%20%20when%20Wednesday%20then%20Weekday%3A%3AWednesday%0A%20%20%20%20when%20Thursday%20then%20Weekday%3A%3AThursday%0A%20%20%20%20when%20Friday%20then%20Weekday%3A%3AFriday%0A%20%20%20%20when%20Saturday%20then%20nil%0A%20%20%20%20when%20Sunday%20then%20nil%0A%20%20%20%20else%20T.absurd(self)%0A%20%20%20%20end%0A%20%20end%0A%0A%20%20sig%20%7Breturns(T.nilable(Weekend))%7D%0A%20%20def%20to_weekend%0A%20%20%20%20case%20self%0A%20%20%20%20when%20Saturday%20then%20Weekend%3A%3ASaturday%0A%20%20%20%20when%20Sunday%20then%20Weekend%3A%3ASunday%0A%20%20%20%20when%20Monday%20then%20nil%0A%20%20%20%20when%20Tuesday%20then%20nil%0A%20%20%20%20when%20Wednesday%20then%20nil%0A%20%20%20%20when%20Thursday%20then%20nil%0A%20%20%20%20when%20Friday%20then%20nil%0A%20%20%20%20else%20T.absurd(self)%0A%20%20%20%20end%0A%20%20end%0Aend%0A%0Aclass%20Weekday%20%3C%20T%3A%3AEnum%0A%20%20enums%20do%0A%20%20%20%20Monday%20%3D%20new%0A%20%20%20%20Tuesday%20%3D%20new%0A%20%20%20%20Wednesday%20%3D%20new%0A%20%20%20%20Thursday%20%3D%20new%0A%20%20%20%20Friday%20%3D%20new%0A%20%20end%0A%0A%20%20sig%20%7Breturns(DayOfWeek)%7D%0A%20%20def%20to_day_of_week%0A%20%20%20%20case%20self%0A%20%20%20%20when%20Monday%20then%20DayOfWeek%3A%3AMonday%0A%20%20%20%20when%20Tuesday%20then%20DayOfWeek%3A%3ATuesday%0A%20%20%20%20when%20Wednesday%20then%20DayOfWeek%3A%3AWednesday%0A%20%20%20%20when%20Thursday%20then%20DayOfWeek%3A%3AThursday%0A%20%20%20%20when%20Friday%20then%20DayOfWeek%3A%3AFriday%0A%20%20%20%20else%20T.absurd(self)%0A%20%20%20%20end%0A%20%20end%0Aend%0A%0Aclass%20Weekend%20%3C%20T%3A%3AEnum%0A%20%20enums%20do%0A%20%20%20%20Saturday%20%3D%20new%0A%20%20%20%20Sunday%20%3D%20new%0A%20%20end%0A%0A%20%20sig%20%7Breturns(DayOfWeek)%7D%0A%20%20def%20to_day_of_week%0A%20%20%20%20case%20self%0A%20%20%20%20when%20Saturday%20then%20DayOfWeek%3A%3ASaturday%0A%20%20%20%20when%20Sunday%20then%20DayOfWeek%3A%3ASunday%0A%20%20%20%20else%20T.absurd(self)%0A%20%20%20%20end%0A%20%20end%0Aend%0A">→
View full example on sorbet.run</a>

As you can see, this example is significantly more verbose, but it is an
alternative when the type safety is worth the tradeoff.

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

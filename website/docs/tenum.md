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

Sorbet allows using individual enum values in types, especially in union types
and type aliases. For example:

```ruby
RedSuit = T.type_alias { T.any(Suit::Hearts, Suit::Diamonds) }
```

This defines a [type alias](type-aliases.md) `RedSuit` composed of only hearts
and diamonds. (Contrast this with the type `Suit`, composed of all four enum
values.)

## Defining a subset of an enum

We can combine the techniques from the two previous sections to define subsets
of enum values within an enum, as well as checked cast functions that convert
into those subsets safely.

For example:

```ruby
class Suit < T::Enum
  enums do
    Spades = new
    Hearts = new
    Clubs = new
    Diamonds = new
  end

  sig { returns(T.nilable(RedSuit)) }
  def to_red_suit
    case self
    when Spades, Clubs then nil
    else
      self
    end
  end
end

RedSuit = T.type_alias { T.any(Suit::Hearts, Suit::Diamonds) }
```

There are two components to this example:

1.  We've defined the `RedSuit` enum subset with a type alias. Due to
    limitations in `T::Enum`, this type alias must be declared outside of the
    enum itself (we expect to lift this restriction in the future).

1.  We've added a `to_red_suit` instance method on `Suit` which converts that
    enum value to either `nil` if called on a black suit, or itself otherwise.

This `to_red_suit` conversion function could be called like this:

```ruby
sig { params(red_suit: Suit).void }
def takes_red_suit(red_suit)
  # ...
end

sig { params(suit: Suit).void }
def example(suit)
  red_suit = suit.to_red_suit
  if red_suit
    takes_red_suit(red_suit)
  else
    puts("#{suit} was not a red suit")
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20Suit%20%3C%20T%3A%3AEnum%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20enums%20do%0A%20%20%20%20Spades%20%3D%20new%0A%20%20%20%20Hearts%20%3D%20new%0A%20%20%20%20Clubs%20%3D%20new%0A%20%20%20%20Diamonds%20%3D%20new%0A%20%20end%0A%0A%20%20sig%20%7B%20returns%28T.nilable%28RedSuit%29%29%20%7D%0A%20%20def%20to_red_suit%0A%20%20%20%20case%20self%0A%20%20%20%20when%20Spades%2C%20Clubs%20then%20nil%0A%20%20%20%20else%0A%20%20%20%20%20%20self%0A%20%20%20%20end%0A%20%20end%0Aend%0A%0ARedSuit%20%3D%20T.type_alias%20%7B%20T.any%28Suit%3A%3AHearts%2C%20Suit%3A%3ADiamonds%29%20%7D%0A%0Asig%20%7B%20params%28red_suit%3A%20Suit%29.void%20%7D%0Adef%20takes_red_suit%28red_suit%29%0A%20%20%23%20...%0Aend%0A%0Asig%20%7B%20params%28suit%3A%20Suit%29.void%20%7D%0Adef%20example%28suit%29%0A%20%20if%20%28red_suit%20%3D%20suit.to_red_suit%29%0A%20%20%20%20takes_red_suit%28red_suit%29%0A%20%20else%0A%20%20%20%20puts%28%22%23%7Bsuit%7D%20was%20not%20a%20red%20suit%22%29%0A%20%20end%0Aend">→
View full example on sorbet.run</a>

Sorbet knows that `to_red_suit` returns `nil` if the suit was not red, so by
assigning to a local variable and using `if`, Sorbet's
[flow-sensitive type checking](flow-sensitive.md) kicks in and allows using
`red_suit` with the non-nil type `RedSuit` inside the `if` condition.

Note that the structure of the `to_red_suit` method implementation is
intentional: it specifies the examples of what is **not** a red suit, instead of
specifying all the cases of what is a red suit in order to be **safe in the
presence of refactors**.

To outline the refactor-safety of this method, here's an extended explanation.
For our `Suit` example it gets a little contrived, because there are always only
four suits and that's very unlikely to change. But we can pretend anyways:

- If a new enum value is added, maybe called `Stars`, Sorbet will catch that the
  `else` branch has type `T.any(Hearts, Diamonds, Stars)`, which is not a
  subtype of `RedSuit`.

  To treat `Stars` as a red suit, we'd want to update the definition of
  `RedSuit` to mention it. To treat it as not a red suit, we'd want to add it to
  the `when ... nil` statement.

- If an enum value is removed from `RedSuit`, then Sorbet will report that as a
  type error similar to the above. After removing a suit from the type alias,
  the fix would be to explicitly list that removed suit in the `when ... nil`
  statement.

- If an enum value is removed from `Suit` entirely, Sorbet reports this as an
  "Unable to resolve constant" error. The fix will be to either remove the
  reference from the `RedSuit` type alias, or from the `when ... nil` statement
  (depending on what was removed from `Suit`).

(It would be possible to achieve the same effect with
[exhaustiveness on enums](#exhaustiveness), but in this particular case that
ends up being overkill—we can get the same safety guarantees with less
redundancy.)

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
> [Defining a subset of an enum](#defining-a-subset-of-an-enum) section above.
> This section is older, and describes workarounds relevant before that section
> above existed. We include this section here mostly for inspiration (the ideas
> in this section are not discouraged, just verbose).

In addition to [defining a subset of an enum](#defining-a-subset-of-an-enum)
with type aliases and conversion methods, there are two other ways to define one
enum as a subset of another:

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

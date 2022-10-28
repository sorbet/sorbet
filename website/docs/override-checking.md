---
id: override-checking
title: Override Checking
---

Sorbet supports method override checking. These checks are implemented as `sig`
annotations:

- `overridable` means children can override this method
- `override` means this method overrides a method on its parent (or ancestor),
  which may or may not be an abstract or interface method
- `abstract` means this method is abstract (has no implementation) and must be
  implemented by being overridden in all concrete subclasses.

These annotations can be chained, for example `.override.overridable` lets a
grandchild class override a concrete implementation of its parent.

Use this table to track when annotations can be used, although the error
messages are the canonical source of truth. ✅ means "this pairing is allowed"
while ❌ means "this is an error".

> Below, `standard` (for the child or parent) means "has a `sig`, but has none
> of the special modifiers."

| ↓Parent \ Child → | no sig | `standard` | `override` |
| ----------------- | :----: | :--------: | :--------: |
| no sig            |   ✅   |     ✅     |     ✅     |
| `standard`        |   ✅   |     ✅     |     ❌     |
| `overridable`     |   ✅   |     ❌     |     ✅     |
| `override`        |   ✅   |     ❌     |     ✅     |
| `abstract`        |   ✅   |     ❌     |     ✅     |

Some other things are checked that don't fit into the above table:

- It is an error to mark a method `override` if the method doesn't actually
  override anything.
- If the implementation methods are inherited--from either a class or mixin--the
  methods don't need the `override` annotation.

Note that the **absence** of `abstract` or `overridable` does **not** mean that
a method is never overridden. To declare that a method can never be overridden,
look into [final methods](final.md).

## A note on variance

When overriding a method, the override must accept at least all the same things
that the parent method accepts, and return at most what the parent method
returns but no more.

This is very abstract so let's make it concrete with some examples:

```ruby
class Parent
  extend T::Sig

  sig {overridable.params(x: T.any(Integer, String)).void}
  def takes_integer_or_string(x); end
end

class Child < Parent
  sig {override.params(x: Integer).void}
  def takes_integer_or_string(x); end # error
end
```

This code has an error because the child class overrides
`takes_integer_or_string` but narrows the input type. It's important to reject
overrides like this, because otherwise Sorbet would not be able to catch errors
like this:

```ruby
sig {params(parent: Parent).void}
def example(parent)
  parent.takes_integer_or_string('some string')
end

example(Child.new) # throws at runtime!
```

In this example, since `Child.new` is an instance of `Parent` (via inheritance),
Sorbet allows call to `example`. Inside `example`, Sorbet assumes that it is
safe to call all methods on `Parent`, regardless of whether they're implemented
by `Parent` or `Child`.

Since `Child#takes_integer_or_string` has been defined in a way that breaks that
contract that it's "at least as good" as the parent class definition, Sorbet
must report an error where the invalid override happens.

When considering that the return type is "at least as good" as the parent, the
subtyping relationship is flipped. Here's an example of incorrect return type
variance:

```ruby
class Parent
  extend T::Sig

  sig {overridable.returns(Numeric)}
  def returns_at_most_numeric; end
end

class Child < Parent
  sig {override.returns(T.any(Numeric, String))}
  def returns_at_most_numeric; end # error
end
```

In this example, the `Parent` definition declares that `returns_at_most_numeric`
will only ever return at most an `Numeric`, so that all callers will be able to
assume that they'll only be given an `Numeric` back (including maybe a subclass
of `Numeric`, like `Integer` or `Float`), but never something else, like a
`String`. So the above definition of `Child#returns_at_most_numeric` is an
invalid override, because it attempts to widen the method's declared return type
to something wider than what the parent specified.

## What if I really want the child method to narrow the type?

The most common place where compatible overrides are difficult or frustrating to
maintain is with arguments. It's common to encounter a scenario where multiple
classes conform to some interface where each class knows that it will only ever
be called with a certain argument type, like this:

```ruby
class DogFood; end
class CatFood; end

class Dog
  sig {params(food: DogFood).void}
  def feed(food); end
end

class Cat
  sig {params(food: CatFood).void}
  def feed(food); end
end
```

A naive approach to extract an interface for the `feed` method might look like
this, which has problems:

```ruby
class DogFood; end
class CatFood; end

module Pet
  extend T::Helpers
  interface!
  # Warning: this `T.any` is faulty, and leads to override checking errors
  sig {abstract.params(food: T.any(DogFood, CatFood)).void}
  def feed(food); end
end

class Dog
  include Pet
  sig {override.params(food: DogFood).void}
  def feed(food); end # error: DogFood is not a supertype of T.any(DogFood, CatFood)
end

class Cat
  include Pet
  sig {override.params(food: CatFood).void}
  def feed(food); end # error: CatFood is not a supertype of T.any(DogFood, CatFood)
end
```

In cases like this, what we actually want, instead of using a `T.any` in the
interface, is to define the `Pet` interface as a [generic class](generics.md)
using `type_member`:

```ruby
class DogFood; end
class CatFood; end

module Pet
  extend T::Helpers
  interface!
  # (1) Pulls in the `type_member` helper
  extend T::Generic
  # (2) Define `Pet` as a generic interface
  FoodType = type_member

  # (3) Use the `FoodType` generic type variable here
  sig {abstract.params(food: FoodType).void}
  def feed(food); end
end

class Dog
  include Pet
  extend T::Generic
  # (4) Declare that Dog implements the Pet interface, subject to the constraint
  #     that FoodType is always DogFood
  FoodType = type_member {{fixed: DogFood}}

  # (5) Use FoodType generic variable in the method.
  #     Because it's the same generic variable as in the abstract method,
  #     `feed` is now a valid override.
  sig {override.params(food: FoodType).void}
  def feed(food); end
end

# same thing for Cat
class Cat
  include Pet
  extend T::Generic
  FoodType = type_member {{fixed: CatFood}}

  sig {override.params(food: FoodType).void}
  def feed(food); end
end
```

[→ View full example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Module%3B%20include%20T%3A%3ASig%3B%20end%0A%0Aclass%20DogFood%3B%20end%0Aclass%20CatFood%3B%20end%0A%0Amodule%20Pet%0A%20%20extend%20T%3A%3AHelpers%0A%20%20interface!%0A%20%20%23%20%281%29%20Pulls%20in%20the%20%60type_member%60%20helper%0A%20%20extend%20T%3A%3AGeneric%0A%20%20%23%20%282%29%20Define%20%60Pet%60%20as%20a%20generic%20interface%0A%20%20FoodType%20%3D%20type_member%0A%0A%20%20%23%20%283%29%20Use%20the%20%60FoodType%60%20generic%20type%20variable%20here%0A%20%20sig%20%7Babstract.params%28food%3A%20FoodType%29.void%7D%0A%20%20def%20feed%28food%29%3B%20end%0Aend%0A%0Aclass%20Dog%0A%20%20include%20Pet%0A%20%20extend%20T%3A%3AGeneric%0A%20%20%23%20%284%29%20Declare%20that%20Dog%20implements%20the%20Pet%20interface%2C%20subject%20to%20the%20constraint%0A%20%20%23%20%20%20%20%20that%20FoodType%20is%20always%20DogFood%0A%20%20FoodType%20%3D%20type_member%20%7B%7Bfixed%3A%20DogFood%7D%7D%0A%0A%20%20%23%20%285%29%20Use%20FoodType%20generic%20variable%20in%20the%20method.%0A%20%20%23%20%20%20%20%20Because%20it's%20the%20same%20generic%20variable%20as%20in%20the%20abstract%20method%2C%0A%20%20%23%20%20%20%20%20%60feed%60%20is%20now%20a%20valid%20override.%0A%20%20sig%20%7Boverride.params%28food%3A%20FoodType%29.void%7D%0A%20%20def%20feed%28food%29%3B%20end%0Aend%0A%0A%23%20same%20thing%20for%20Cat%0Aclass%20Cat%0A%20%20include%20Pet%0A%20%20extend%20T%3A%3AGeneric%0A%20%20FoodType%20%3D%20type_member%20%7B%7Bfixed%3A%20CatFood%7D%7D%0A%0A%20%20sig%20%7Boverride.params%28food%3A%20FoodType%29.void%7D%0A%20%20def%20feed%28food%29%3B%20end%0Aend)

This approach has some key benefits:

- All the override methods are compatible overrides.

- The `Pet` interface is explicit about what the relationship is between the
  implementing class and the type of the `feed` method. For example,

  ```ruby
  sig do
    params(
      food: T.any(DogFood, CatFood),
      pet: Pet
    )
    .void
  end
  def give_food_to_pet(food, pet)
    # Warning: does not guarantee `food` is the right type for `pet`
    pet.feed(food)
  end
  ```

  This method, assuming the original (not generic) implementation of `Pet`
  doesn't actually check whether it's okay to give `food` to `pet`.

  In the generic example, it says that `Pet` is a generic class without type
  arguments, which essentially forces us to rewrite this method in a way that
  guarantees that the `food` type matches the `pet` type.

  ```ruby
  sig do
    type_parameters(:Food)
      .params(
        food: T.type_parameter(:Food),
        pet: Pet[T.type_parameter(:Food)]
      )
      .void
  end
  def give_food_to_pet(food, pet)
    pet.feed(food)
  end

  give_food_to_pet(DogFood.new, Dog.new)
  give_food_to_pet(CatFood.new, Dog.new) # error!
  ```

  [→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0Aclass%20Module%3B%20include%20T%3A%3ASig%3B%20end%0A%0Aclass%20DogFood%3B%20end%0Aclass%20CatFood%3B%20end%0A%0Amodule%20Pet%0A%20%20extend%20T%3A%3AHelpers%0A%20%20interface!%0A%20%20%23%20%281%29%20Pulls%20in%20the%20%60type_member%60%20helper%0A%20%20extend%20T%3A%3AGeneric%0A%20%20%23%20%282%29%20Define%20%60Pet%60%20as%20a%20generic%20interface%0A%20%20FoodType%20%3D%20type_member%0A%0A%20%20%23%20%283%29%20Use%20the%20%60FoodType%60%20generic%20type%20variable%20here%0A%20%20sig%20%7Babstract.params%28food%3A%20FoodType%29.void%7D%0A%20%20def%20feed%28food%29%3B%20end%0Aend%0A%0Aclass%20Dog%0A%20%20include%20Pet%0A%20%20extend%20T%3A%3AGeneric%0A%20%20%23%20%284%29%20Declare%20that%20Dog%20implements%20the%20Pet%20interface%2C%20subject%20to%20the%20constraint%0A%20%20%23%20%20%20%20%20that%20FoodType%20is%20always%20DogFood%0A%20%20FoodType%20%3D%20type_member%20%7B%7Bfixed%3A%20DogFood%7D%7D%0A%0A%20%20%23%20%285%29%20Use%20FoodType%20generic%20variable%20in%20the%20method.%0A%20%20%23%20%20%20%20%20Because%20it's%20the%20same%20generic%20variable%20as%20in%20the%20abstract%20method%2C%0A%20%20%23%20%20%20%20%20%60feed%60%20is%20now%20a%20valid%20override.%0A%20%20sig%20%7Boverride.params%28food%3A%20FoodType%29.void%7D%0A%20%20def%20feed%28food%29%3B%20end%0Aend%0A%0A%23%20same%20thing%20for%20Cat%0Aclass%20Cat%0A%20%20include%20Pet%0A%20%20extend%20T%3A%3AGeneric%0A%20%20FoodType%20%3D%20type_member%20%7B%7Bfixed%3A%20CatFood%7D%7D%0A%0A%20%20sig%20%7Boverride.params%28food%3A%20FoodType%29.void%7D%0A%20%20def%20feed%28food%29%3B%20end%0Aend%0A%0Asig%20do%0A%20%20type_parameters%28%3AFood%29%0A%20%20%20%20.params%28%0A%20%20%20%20%20%20food%3A%20T.type_parameter%28%3AFood%29%2C%0A%20%20%20%20%20%20pet%3A%20Pet%5BT.type_parameter%28%3AFood%29%5D%0A%20%20%20%20%29%0A%20%20%20%20.void%0Aend%0Adef%20give_food_to_pet%28food%2C%20pet%29%0A%20%20pet.feed%28food%29%0Aend%0A%0Agive_food_to_pet%28DogFood.new%2C%20Dog.new%29%0Agive_food_to_pet%28CatFood.new%2C%20Dog.new%29%0A%0A)

For more information about designing generic interfaces, see
[Generic Classes and Methods](generics.md).

## Escape hatches for override checking

When confronted with an override checking error, the first reaction should
always be to fix the error. As described in the previous sections, incompatible
overrides by a child class break the contract established by the parent class.

If you've exhausted all other options and simply need to silence the error to
make progress, there are two main approaches.

### Use `T.untyped`

Changing either the parent method or the child method type to `T.untyped` will
essentially silence the type error for that position. `T.untyped` is both a
supertype and subtype of all types, which we've previously described as a
[double-edged sword](gradual.md).

If the `T.untyped` is placed on the parent class, it will silence the
incompatible override warnings for **all** child classes, as well as opting out
of static type checking for all uses of the parent class.

If the `T.untyped` is placed on the child class, it will be limited in effect to
just that class.

```ruby
# -- WARNING: Uses T.untyped to opt out of static override checking! --

class Parent
  extend T::Sig

  sig {overridable.returns(Numeric)}
  def returns_at_most_numeric; end
end

class Child < Parent
  sig {override.returns(T.untyped)}
  def returns_at_most_numeric; end # no error, because of T.untyped
end
```

### Use `override(allow_incompatible: true)`

Using `T.untyped` can be heavy handed, as it means not only opting out of
override checking, but also out of normal argument or return type checking.

An alternative to **only** silence the override checks while keeping the
incompatible types is to use `override(allow_incompatible: true)`:

```ruby
class Parent
  extend T::Sig

  sig {overridable.returns(Numeric)}
  def returns_at_most_numeric; end
end

class Child < Parent
  sig {override(allow_incompatible: true).returns(T.any(Numeric, String))}
  def returns_at_most_numeric; end # no error, explicitly silenced
end
```

**Again**, reach for this escape hatch sparingly. Every location where override
checking has been silenced is a place where Sorbet could fail to catch an error
that it might otherwise have been able to catch.

## What's next?

- [Final Methods, Classes, and Modules](final.md)

  Learn how to prohibit overriding entirely, both at the method level and the
  class level.

- [Abstract Classes and Interfaces](abstract.md)

  Marking methods as `abstract` and requiring child classes to implement them is
  a powerful tool for code organization and correctness. Learn more about
  Sorbet's support for abstract classes and interfaces.

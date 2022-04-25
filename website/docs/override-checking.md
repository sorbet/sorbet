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

When overriding a method, the override must accept at all the same things that
the parent method accepts, and return at most what the parent method returns but
no more.

This is very abstract so let's make it concrete with some examples:

```ruby
class Parent
  extend T::Sig

  sig {overridable.params(x: T.any(Integer, String)).void}
  def takes_integer_or_string; end
end

class Child < Parent
  sig {override.params(x: Integer).void}
  def takes_integer_or_string; end # error
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

## What's next?

- [Final Methods, Classes, and Modules](final.md)

  Learn how to prohibit overriding entirely, both at the method level and the
  class level.

- [Abstract Classes and Interfaces](abstract.md)

  Marking methods as `abstract` and requiring child classes to implement them is
  a powerful tool for code organization and correctness. Learn more about
  Sorbet's support for abstract classes and interfaces.

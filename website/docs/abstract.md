---
id: abstract
title: Abstract Classes and Interfaces
sidebar_label: Abstract Classes & Interfaces
---

Sorbet supports abstract classes, abstract methods, and interfaces. Abstract
methods ensure that a particular method gets implemented anywhere the class or
module is inherited, included, or extended. An abstract class or module is one
that contains one or more abstract methods. An interface is a class or module
that must have only abstract methods.

Keep in mind:

- `abstract!` can be used to prevent a class from being instantiated.
- Both `abstract!` and `interface!` allow the class or module to have `abstract`
  methods.
- Mix in a module (via `include` or `extend`) to declare that a class implements
  an interface.

> **Note**: Most of the abstract and override checks are implemented statically,
> but some are still only implemented at runtime, most notably variance checks.

## Creating an abstract method

To create an abstract method:

1.  Add `extend T::Helpers` to the class or module (in addition to
    `extend T::Sig`).
1.  Add `abstract!` or `interface!` to the top of the class or module. (_All_
    methods must be abstract to use `interface!`.)
1.  Add a `sig` with `abstract` to any methods that should be abstract, and thus
    implemented by a child.
1.  Declare the method on a single line with an empty body.

```ruby
module Runnable
  extend T::Sig
  extend T::Helpers                                    # (1)
  interface!                                           # (2)

  sig {abstract.params(args: T::Array[String]).void}   # (3)
  def main(args); end                                  # (4)
end
```

## Implementing an abstract method

To implement an abstract method, define the method in the implementing class or
module with an identical signature as the parent, except replacing `abstract`
with `override`.

```ruby
class HelloWorld
  extend T::Sig
  include Runnable

  # This implements the abstract `main` method from our Runnable module:
  sig {override.params(args: T::Array[String]).void}
  def main(args)
    puts 'Hello, world!'
  end
end
```

## Additional Requirements

There are some additional stipulations on the use of `abstract!` and
`interface!`:

- All methods in a module marked as `interface!` must have signatures, and must
  be marked `abstract`.
  - **Note**: this applies to all methods defined within the module, as well as
    any that are included from another module
- A module marked `interface!` can't have `private` or `protected` methods.
- Any method marked `abstract` must have no body. `sorbet-runtime` will take
  care to raise an exception if an abstract method is called at runtime.
- Classes without `abstract!` or `interface!` must implement all `abstract`
  methods from their parents.
- `extend MyAbstractModule` works just like `include MyAbstractModule`, but for
  singleton methods.
- `abstract!` classes cannot be instantiated (will raise at runtime).

## Abstract singleton methods

`abstract` singleton methods on a module are not allowed, as there's no way to
implement these methods.

```ruby
module M
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def self.foo; end
end

M.foo # error: `M.foo` can never be implemented
```

## Interfaces and the `included` hook

A somewhat common pattern in Ruby is to use an `included` hook to mix class
methods from a module onto the including class:

```ruby
module M
  module ClassMethods
    def foo
      self.bar
    end
  end

  def self.included(other)
    other.extend(ClassMethods)
  end

end

class A
  include M
end

# runtime error as `bar` is not defined on A
A.bar
```

This is hard to statically analyze, as it involves looking into the body of the
`self.included` method, which might have arbitrary computation. As a compromise,
Sorbet provides a new construct: `mixes_in_class_methods`. At runtime, it
behaves as if we'd defined `self.included` like above, but will declare to `srb`
statically what module is being extended.

We can update our previous example to use `mixes_in_class_methods`, which lets
Sorbet catch the runtime error about `bar` not being defined on `A`:

```ruby
# typed: true
module M
  extend T::Helpers
  interface!

  module ClassMethods
    extend T::Sig
    extend T::Helpers
    abstract!

    sig {void}
    def foo
      bar
    end

    sig {abstract.void}
    def bar; end
  end

  mixes_in_class_methods(ClassMethods)
end

class A # error: Missing definition for abstract method
  include M

  extend T::Sig

  sig {override.void}
  def self.bar; end
end

# Sorbet knows that `foo` is a class method on `A`
A.foo
```

## What's next?

- [Override Checking](override-checking.md)

  Sorbet has more ways to check overriding than just whether an abstract method
  is implemented in a child. See this doc to learn about the ways to declare
  what kinds of overriding should be allowed.

- [Sealed Classes and Modules](sealed.md)

  Abstract classes and interfaces are frequently used with sealed classes to
  recreate a sort of "algebraic data type" in Sorbet.

---
id: abstract
title: Abstract Classes and Interfaces
---

Sorbet supports abstract classes, abstract methods, and interfaces.

> **Note**: Much of the support for abstract classes, methods, and interfaces is
> only implemented using the runtime system. At Stripe, these checks are
> enforced with a wholesome test that loads up all Stripe's code and checks
> that these invariants hold. Support for these checks in the static system is
> planned for the future.

Abstract methods ensure that a particular method gets implemented
anywhere the class or module is inherited, included, or extended.

Keep in mind:
- `abstract!` can be used to prevent a class from being instantiated.
- Both either `abstract!` and `interface!` allow the class or module to
  have `abstract` methods.
- Mixin a module (via `include` or `extend`) to declare that a class
  implements an interface.


## Creating an abstract method

To create an abstract method:
1.  Add `extend T::Helpers` to the class or module (as well as `extend T::Sig`).
1.  Add `abstract!` or `interface!` to the top of the class or module. (*All*
    methods must be abstract to use `interface!`.)
1.  Add a `sig` with `abstract` to any methods that should be abstract.
1.  Define the method on a single line with an empty body.

```ruby
module Runnable
  extend T::Sig
  extend T::Helpers                                    # (1)
  interface!                                           # (2)

  sig {abstract.params(args: T::Array[String]).void}   # (3)
  def main(args); end                                  # (4)
end
```

> **Note**: the class must extend `T::Helpers` to use `abstract!` or
> `interface!`.

## Implementing an abstract method

Define the method in the implementing class or module with the same signature as the
parent, changing `abstract` to `implementation`. 
Sorbet cannot check types on the child, statically or at runtime, unless the
signatures match.

```ruby
class HelloWorld
  extend T::Sig
  include Runnable

  sig {implementation.params(args: T::Array[String]).void}
  def main(args)
    puts 'Hello, world!'
  end
end
```

- - -

## Additional Requirements

Use of `abstract!` and `interface!` requires:

- All methods in a module marked as `interface!` have signatures
- The methods of classes or modules marked `interface!` must all be marked `abstract`
  - **Note**: this applies to all methods defined within the module, as well as
    any that are included from another module
- The `interface!` can't have `private` or `protected` methods
- `abstract` self-methods on a module are not allowed, as there's no way to
  implement these methods
- `abstract` methods will raise an error when called in the runtime
- Classes without `abstract!` or `interface!` must implement all `abstract`
  methods from their parents
- `extend MyAbstractModule` works just like `include MyAbstractModule`, but for self-methods
- `abstract!` classes cannot be instantiated


- - -

## Interfaces and the `included` hook

A somewhat common pattern in Ruby is to use an `included` hook to mix class
methods from the module onto the including class:

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
A.foo
```

To be more explicit, Sorbet provides a new construct: `mixes_in_class_methods`.
At runtime, it behaves just like the above, but statically `srb` will know what
class methods are being defined.

Updating the previous example to use `mixes_in_class_methods`, Sorbet catches
the runtime error about `bar` not being defined on `A`.

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

  sig {implementation.void}
  def self.bar; end
end

# Sorbet knows that `foo` is a class method on `A`
A.foo
```

- - -



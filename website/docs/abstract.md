---
id: abstract
title: Abstract Classes and Interfaces
---

> TODO(jez) This page is still a fragment. Contributions welcome!

Sorbet has some support for abstract classes, abstract methods, and interfaces.

> **Note**: Much of the support for abstract classes, methods, and interfaces is
> only implemented using the runtime system. At Stripe, these checks are
> enforced with a "wholesome test" that loads up all Stripe's code and checks
> that these invariants hold. Support for these checks in the static system is
> planned for the future.

Abstract methods let you ensure that a particular method gets implemented
anywhere your class / module is inherited, included, or extended.

## Creating an abstract method

1.  Add `extend T::Helpers` to your class or module.
1.  Add `abstract!` or `interface!` to the top of your class or module
    - (`interface!` requires that *all* methods be abstract)
1.  Add a `sig` with `abstract` to any methods that should be abstract
1.  Define the method on a single line with an empty body.

Example:

```ruby
module Runnable
  extend T::Helpers                                    # (1)
  interface!                                           # (2)

  sig {abstract.params(args: T::Array[String]).void}   # (3)
  def main(args); end                                  # (4)
end
```

## Implementing an abstract method

Define the method like normal in the implementing class or module. The parent
`sig` must be copied to the child, or the types will not be checked on the
child, neither statically nor at runtime.

Example:

```ruby
class HelloWorld
  include Runnable

  sig {implementation.params(args: T::Array[String]).void}
  def main(args)
    puts 'Hello, world!'
  end
end
```

- - -

This is a dump of some other notes, and is somewhat redundant with the above guide.

<!-- TODO(jez) This doc violates our style-guide.md w.r.t. usage of "you". -->

- You can `extend T::Helpers` to get access to `abstract!` and `interface!`
- You can use `abstract!` to prevent a class from being instantiated.
- You can use either `abstract!` or `interface!` to allow the class or module to
  have `abstract` methods.
- You can mixin a module (`include` / `extend`) to declare that a class
  implements an interface.


<!-- TODO(jez) It's possible that some of these points about overrides belong in a different doc instead of here. -->

This is somewhat unrelated to abstract methods specifically, but Sorbet also has
method override checking. These checks are implemented as builder methods you
can attach to a `sig`:

- `overridable`: children can override this method
- `override`: this class overrides a method on its parent (or ancestor)
- `abstract`: this method is "abstract" (has no implementation) and must be
  implemented by being overridden in all concrete subclasses.
- `implementation`: this method implements an abstract method

Note that you can chain these, so you can have `.implementation.overridable` to
let a grand child class override a concrete implementation of the parent.

It can be confusing when some of these builders are required and when they
aren't. The error messages are the canonical source of truth, but at a glance
here's a table. ✅ means "this pairing is allowed" while ❌ means "this is an
error" (remember, most of these are runtime-only errors currently).

> Below, `standard` means "has a sig, but has none of the special modifiers."


| ↓Child \ Parent → | no sig | `standard` | `overridable` | `override` | `implementation` | `abstract` |
| ----------------- | ------ | ---------- | ------------- | ---------- | ---------------- | ---------- |
| no sig            | ✅     | ✅         | ✅            | ✅         | ✅               | ✅         |
| `standard`        | ✅     | ✅         | ❌            | ❌         | ❌               | ❌         |
| `override`        | ✅     | ❌         | ✅            | ✅         | ❌               | ❌         |
| `implementation`  | ❌     | ❌         | ❌            | ❌         | ❌               | ✅         |

> Note: if the implementation methods come from inheritance (either class or
> mixin) they don't need `implementation`.

Other checks:

- all methods in `interface!` need `sig`
- `interface!` → all method are `abstract`
  - both if all methods are defined in current module
  - and also if some methods were defined in an `included` module
- an `interface!` can't have `private` or `protected` methods.
- the type must match when overriding / implementing a parent method
- `abstract` +  `self.` methods on a module are not allowed (there's no way to
  implement these methods)
- `abstract` methods will raise when called in the runtime
- classes without `absract!` or `interface!` must implement all `abstract`
  methods from their parents
- `extend MyAbstractModule` works just like `include MyAbstractModule`, but for `self.` methods
- `abstract!` cannot be instantiated


- - -

<!-- TODO(jez) It's possible that mixes_in_class_methods belongs a different doc instead of here. -->

A somewhat common pattern in Ruby is to use an `included` hook to mix class
methods from the module onto the including class:

```ruby
# ...

def included(other)
  other.extend(self)
end
```

Instead of attempting understand this specific pattern, exposes a new construct:
`mixes_in_class_methods`. At runtime, it behaves just like the above, but
statically `sorbet` will know what class methods are being defined.

Here's an example:

```ruby
module A
  extend T::Helpers
  interface!

  module ClassMethods
    extend T::Helpers
    abstract!

    sig {abstract.returns(Integer)}
    def foo; end
  end

  mixes_in_class_methods(ClassMethods)
end

class B # error: Missing definition for abstract method
  include A
end

# Sorbet knows that `foo` is a class method on `B`
B.foo
```


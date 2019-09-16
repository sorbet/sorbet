---
id: final
title: Final Methods, Classes, and Modules
sidebar_label: Final Methods & Classes
---

Sorbet supports marking methods, classes, and modules "final", which limits how
they can be overridden and extended, making some patterns easier to reason about
and depend on.

In this doc we'll answer:

- What does it mean for a method to be final?
- What about final classes and modules?
- What are some use cases for final method and classes?

Before we get into the in-depth details, here's a full snippet that demonstrates
the syntax for all these features at once:

```ruby
# typed: true
require 'sorbet-runtime'

# (0) One-time setup for final:
T::Configuration.enable_final_checks_on_hooks

class A
  extend T::Sig      # (Brings `sig` into scope)
  extend T::Helpers  # (Brings `final!` into scope)

  final!             # (1) Final class (can't be subclassed)

  sig(:final) {void} # (2) Final method (can't be overridden / redefined)
  def foo; end
end
```

## Final methods

Final methods can't be overridden or redefined. This is a powerful guarantee: it
means that inheritance can't affect what code will be run when calling a method
on a class.

For example, it might be a good idea to mark methods that acquire a lock to a
shared resource as final so that the class can't be subclassed and tampered with
to avoid acquiring the locks. Marking these methods final reduces the scope of
correctness the original author has to worry about.

To start using final, there's some one-time, per-project setup:

1. Call `T::Configuration.enable_final_checks_on_hooks` at least once.

Once that's done, making a method final is straightforward:

2. Use the syntax `sig(:final)` above a method to declare it final:

Here's a full example:

```ruby
require 'sorbet-runtime'
# (1) Call this once per project, ideally right after `require 'sorbet-runtime'`
T::Configuration.enable_final_checks_on_hooks

module HasFinalMethod
  extend T::Sig

  # (2) The special `sig(:final)` syntax declares this method final:
  sig(:final) {void}
  def foo; end
end
```

Final methods must be defined exactly once on their enclosing module or class,
and can never be overridden in a subclass or module. For example, here are
things that are errors:

> **Note**: Some of the features of final methods are only implemented in the
> runtime system. Support for these checks in the static system is planned for
> the future.

```ruby
T::Configuration.enable_final_checks_on_hooks

class Parent
  extend T::Sig
  sig(:final) {void}
  def foo; end

  def foo; end # error: Redefining final method
end

class Child < Parent
  def foo; end # error: Overriding final method
end
```

Note in particular that stubbing a method is considered a method redefinition in
the Ruby runtime, and is therefore not allowed on final methods. This is a
feature, and is unlikely to change:

```ruby
class MyFeature
  extend T::Sig
  sig(:final) {returns(T::Boolean)}
  def self.enabled; false; end
end

# error: Redefining final method
MyFeature.stubs(:enabled).returns(true)
```

## Final classes and modules

In the same spirit as final methods, final classes cannot be subclassed, and
final modules cannot be included or extended. But more than that, every method
in a final class or module must be made into a final method.

It might seem redundant to require final classes to mark all methods final too,
("How could a method be overridden in a subclass if the act of subclassing is
prohibited?") but the answer is ([like many](gradual.md)) that this protects
against untyped code. This guarantees that untyped or ignored code can't
redefine methods at runtime in a class that is marked `final!` statically.

Final classes are good for "plain old data" classes, as well as classes or
modules that are meant be no more than namespaces for functions (i.e., those
which don't want to have to worry about being affected by inheritance).

As mentioned above, to start using final, there's some one-time, per-project
setup:

1. Call `T::Configuration.enable_final_checks_on_hooks` at least once.

And once that's done, making a class or module final is straightforward:

2. Add `extend T::Helpers` to the class (or module) body.
3. Call `final!` at the top-level of the class (or module).

Here's a full example:

```ruby
require 'sorbet-runtime'
# (1) Call this once per project, ideally right after `require 'sorbet-runtime'`
T::Configuration.enable_final_checks_on_hooks

class FinalParentClass
  # (2) Bring `final!` into scope:
  extend T::Helpers

  # (3) Use `final!` to declare this class final
  final!

  # (4) If there are any methods in this class, they must be final
  sig(:final) {void}
  def foo; end
end

# (5) Final classes can't be subclassed:
class ChildClass < FinalParentClass; end # error!

module FinalModule
  extend T::Helpers
  final!
end

class MixesInFinalModule
  # (6) Final modules can't be included or extended:
  include FinalModule # error!
  extend  FinalModule # error!
end
```

## A note on syntax

> **Note**: This section is rather technical and is not relevant to the question
> of "how do I use final methods and classes?"

The syntax for final methods is different from the syntax for things like
abstract methods:

```ruby
# These attributes are inside the block:
sig {overridable.void}
sig {override.void}
sig {abstract.void}
sig {implementation.void}

# But this one is outside the block:
sig(:final) {void}
```

The reason for this difference is that this gives us stronger runtime
guarantees. In general, any Ruby method might be overridden at any time, with no
warning. So the **absence** of an `overridable` or `abstract` attribute on a
method signature does not guarantee that a method is never overridden. Given
these circumstances, it's fine for override / abstract checks to be done lazily,
because the stakes for eliding an error for them wrong is relatively low.

But for final methods, the stakes are higher for missing an error. We aspire to
have Sorbet's type annotations be strong enough to one day enable code to
**run** faster. In particular, known final methods can be made to execute much
faster than otherwise, because calling the method shouldn't need to do complex
virtual dispatch. Also, final methods often must be final from a correctness
standpoint (recall our earlier example of methods which acquire locks).

Thus, we've taken special care to make sure final methods can't be tampered
with, even in the runtime. The `sig(:final)` syntax is an artifact of this
implementation (method signatures are usually lazily evaluated, to avoid
circular constant dependencies and make code load faster).

By moving the `final` attribute outside the block, `sorbet-runtime` can learn
that a method is final without having to force the block to execute.

## More on `T::Configuration.enable_final_checks_on_hooks`

Some runtime final checks can only be checked by installing global monkey
patches. Users who want 100% confidence that final methods and classes behave
like described in this doc will want to opt into these checks. But for example
libraries or other users may not want to enable these monkey patches for
interoperability concerns.

Specifically, calling `enable_final_checks_on_hooks` will install runtime hooks
on `Module#included`, `Module#extended`, and `Class#inherited`.

If these checks are enabled, any classes or modules that define their own
`included`, `extended`, or `inherited` hooks should take care to **always** call
`super`!

If these checks are not enabled, then some but not all of the runtime checks for
final will be run. For instance, the following example violates the requirements
of final, but the violation is not reported at runtime:

```ruby
module M
  extend T::Sig
  sig(:final) {returns(Integer)}
  def foo; 1; end
end

class C
  include M

  def foo; 2; end
end

puts C.new.foo
```

At runtime, this does not raise and prints 2, showing that a final method has
been overridden. This is why we strongly recommend calling
`T::Configuration.enable_final_checks_on_hooks` before using final.

## Known static limitations

Some of the guarantees of final are only implemented at runtime. This section
aims to document those which are most likely to be encountered in normal usage:

1.  When a method is redefined with matching arguments, as in:

    ```ruby
    # typed: true
    T::Configuration.enable_final_checks_on_hooks

    module Bad
      extend T::Sig
      sig(:final) {void}
      def foo; end
      def foo; end # runtime-only error: Redefining final method
    end
    ```

1.  When a method is overridden via including two conflicting modules, as in:

    ```ruby
    # typed: true
    T::Configuration.enable_final_checks_on_hooks

    module A
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end

    module B
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end

    module Bad
      include A
      include B # runtime-only error: Overriding final method from `A`
    end
    ```

## What's next?

- [Sealed Classes and Modules](sealed.md)

  Sealed classes are similar to final classes in that they restrict who's
  allowed to subclass a given class, but they can be used to enforce slightly
  different guarantees.

- [Abstract Classes and Interfaces](abstract.md)

  Marking methods as `abstract` and requiring child classes to implement them is
  a powerful tool for code organization and correctness. Learn more about
  Sorbet's support for abstract classes and interfaces.

- [Override Checking](override-checking.md)

  When override checking is **desired**, Sorbet has ways to declare that intent
  as well.

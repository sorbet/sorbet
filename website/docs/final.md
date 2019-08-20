---
id: final
title: Final Methods, Classes, and Modules
sidebar_label: Final Methods & Classes
---

Sorbet supports final methods, classes, and modules.

> **Note**: Some of the support for final is only implemented using the runtime
> system. Support for these checks in the static system is planned for the
> future.

## Final methods

### Defining a final method

1. Call `T::Configuration.enable_final_checks_on_hooks` at least once.
1. Add `extend T::Sig` to the class or module.
1. Add a sig for the method, with the additional modifier `:final` passed as an
   argument to `sig`.
1. Define the method as usual.

```ruby
T::Configuration.enable_final_checks_on_hooks

module HasFinalMethod
  extend T::Sig
  sig(:final) {void}
  def foo; end
end
```

### Semantics of final methods

Final methods must be defined exactly once on their enclosing module or class.

```ruby
T::Configuration.enable_final_checks_on_hooks

module HasFinalMethod
  extend T::Sig
  sig(:final) {void}
  def foo; end

  def foo; end # error
end
```

Final methods must not be overridden by any class or module that includes,
extends, or inherits the class or module in which the final method was defined.

```ruby
T::Configuration.enable_final_checks_on_hooks

module HasFinalMethod
  extend T::Sig
  sig(:final) {void}
  def foo; end
end

module BadModule
  include HasFinalMethod
  def foo; end # error
end
```

## Final classes and modules

### Defining a final class or module

1. Call `T::Configuration.enable_final_checks_on_hooks` at least once.
1. Add `extend T::Helpers` in the body of the class or module.
1. Call `final!` in the body of the class or module.

```ruby
T::Configuration.enable_final_checks_on_hooks

module FinalModule
  extend T::Helpers
  final!
end

class FinalClass
  extend T::Helpers
  final!
end
```

### Semantics of final classes and modules

Final classes and modules must not be included, extended, or inherited.

```ruby
T::Configuration.enable_final_checks_on_hooks

module FinalModule
  extend T::Helpers
  final!
end

module BadModule
  include FinalModule # error
  extend FinalModule # error
end

class FinalClass
  extend T::Helpers
  final!
end

class BadClass < FinalClass; end # error
```

Every method defined in a final class or module must be explicitly declared as
final.

```ruby
T::Configuration.enable_final_checks_on_hooks

module FinalModule
  extend T::Helpers
  final!

  sig(:final) {void}
  def good_final_sig; end

  sig {void}
  def bad_regular_sig; end # error

  def bad_no_sig; end # error
end
```

## Final method syntax

Note that the syntax for final methods is different from the usual syntax for
declaring things as abstract, override, implementation, etc.

```ruby
sig {abstract.void}
sig {override.void}
sig {implementation.void}
sig(:final) {void}
```

The main reason for this difference is that we want to know that a method is
final before running the block inside the sig.

If the syntax for marking a method final was to have a `final.` builder put
inside the block to sig, similar to `abstract.`, `override.`, etc, then, if we
wanted to know whether a method has been marked final or not, then we would need
to run the block in the `sig`.

The problem is, we would like to know whether a method is final or not in the
runtime "immediately", i.e. before the method has even been called. This is at
odds with the semantics of the block to `sig`: the `sig` block is lazy, and the
code inside it is not run until the first time the method is called.

That is, if we wanted to use a `final.` builder inside the block to `sig.`, and
also wanted to know whether a method is final "immediately", then we would be
forced to run the `sig` block immediately and not defer it until later. But, if
we were to force the sig block to be run immediately after saying "sig", then we
would lose the benefits of having the sig block be lazy.

We intentionally want to keep the block to sig lazy, because that means, for
instance, you may reference constants inside the `sig` block that aren't yet
defined but will be defined later.

## Enabling all of the runtime checks

Before using final in a project, we recommend opting in to all of the runtime
checks.

After requiring `sorbet-runtime`, but before declaring anything as final, call
`T::Configuration.enable_final_checks_on_hooks`.

This will install some runtime hooks on `Module#included`, `Module#extended`,
and `Class#inherited`, to allow us to do more runtime checking for correct final
usage.

If these checks are enabled, any classes or modules that define their own
`included`, `extended`, or `inherited` hooks should take care to always call
`super`.

If these checks are not enabled, then some but not all of the runtime checks for
final will be run. For instance, in the following example, we violate the
requirements of final, but the violation is not a runtime error because we did
not call `T::Configuration.enable_final_checks_on_hooks`:

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

At runtime, this does not raise and prints 2, showing that we have violated the
guarantees of final. This is why we strongly recommend calling
`T::Configuration.enable_final_checks_on_hooks` before using final.

## Limitations of the statics

The following violations of the semantics of "final" are currently not caught by
the static checks (they are only caught by the runtime checks):

1.  When a method is redefined, as in:

    ```ruby
    # typed: true
    T::Configuration.enable_final_checks_on_hooks

    module Bad
      extend T::Sig
      sig(:final) {void}
      def foo; end
      def foo; end
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
      include B
    end
    ```

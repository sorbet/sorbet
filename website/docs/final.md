---
id: final
title: Final Methods, Classes, and Modules
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

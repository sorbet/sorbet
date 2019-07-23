---
id: final
title: Final Methods, Classes, and Modules
---

Sorbet supports final methods, classes, and modules.

> **Note**: Some of the support for final is only implemented using the runtime
> system. Support for these checks in the static system is planned for the
> future.

## Definitions

- A final method `m` on a class or module `A` is a method such that
  - any class or module that includes, extends, or inherits `A` may not define
    its own method `m`
  - `A` itself may only define `m` exactly once
- A final class or module `F` is a class or module such that
  - no class or module may include, extend, or inherit `F`
  - every method defined on `F` is final

## Enabling all of the runtime checks

Before using final in a project, we recommend opting in to all of the runtime
checks.

After requiring `sorbet-runtime`, but before declaring anything as final, call
`T::Configuration.enable_final_checks_on_hooks`.

This will install some runtime hooks on `Module#included`, `Module#extended`,
and `Class#inherited`, to allow us to do more runtime checking for correct final
usage.

If these checks are enabled, any classes or modules that declare their own
`included`, `extended`, or `inherited` hooks should take care to always call
`super`.

## Creating a final method

1.  Add `extend T::Sig` to the class or module.
2.  Add a sig for the method, with the additional modifier `:final` passed as an
    argument to `sig`. (See the discussion on syntax.)
3.  Define the method as usual.

```ruby
module HasFinalMethod
  extend T::Sig       # 1
  sig(:final) {void}  # 2
  def foo; end        # 3
end
```

Note that the syntax for final methods is different from the usual syntax for
declaring things as abstract, override, implementation, etc.

```ruby
sig {abstract.void}
sig {override.void}
sig {implementation.void}
```

The main reason for this difference is that we want to know that a method is
final before running the block inside the sig.

## Creating a final class or module

1. Add `extend T::Helpers` to the class or module.
2. Call `final!` in the class or module.

```ruby
module FinalModule
  extend T::Helpers  # 1
  final!             # 2
end

class FinalClass
  extend T::Helpers  # 1
  final!             # 2
end
```

Note that every method defined in a final class or module must be explicitly
declared as final.

```ruby
module FinalModule
  extend T::Helpers
  final!

  sig(:final) {void}
  def good_final_sig; end

  sig {void}
  def bad_regular_sig; end # error: `FinalModule` was declared as final but its method `bad_regular_sig` was not declared as final

  def bad_no_sig; end # error: `FinalModule` was declared as final but its method `bad_no_sig` was not declared as final
end
```

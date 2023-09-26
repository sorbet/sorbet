---
id: unsupported
title: Unsupported Ruby Features
---

Some features of Ruby Sorbet intentionally does not support. This doc aims to
document the most commonly Ruby features which are not supported in Sorbet
**and** for which Sorbet will not produce an error.

> **Note**: This page is not exhaustive.
>
> If something is missing from this page, it says nothing about whether Sorbet
> supports it, supports it but has a bug in the implementation, or does not ever
> intend to support it.
>
> When in doubt, please
> [open an issue](https://github.com/sorbet/sorbet/issues/new/choose).

## Constant resolution through constant scopes via inheritance

```ruby
class Parent
  X = 1
end

class Child < Parent
  p(X)  # ✅ okay in both
end

p(Child::X)  # in Ruby   => ✅ 1
             # in Sorbet => ❌ error
```

Sorbet does not support constant resolution through inheritance when given an
explicit scope.

### Alternative

Use `Parent::X` instead of `Child::X`

### Why?

- Performance

  Constant resolution is one of the most performance sensitive parts of Sorbet.

- Understandability

  In this case, the code is easier to understand by simply replacing `Child::X`
  with `Parent::X`. This can always be done because Sorbet does not support
  dynamic constant references, so the scope is always known statically.

## `prepend`

```ruby
module WillBePrepended
  def foo
    puts 'WillBePrepended#foo'
  end
end

class Example
  prepend WillBePrepended
  def foo
    puts 'Example#foo'
  end
end

Example.new.foo # => WillBePrepended#foo
```

Sorbet does not model inheritance relationships introduced by `prepend`.

### Alternative

- Usually, static support for `prepend` is not required, even in codebases that
  make heavy use of `prepend`.
- In the rare cases where `prepend` is required, we recommend using
  [escape hatches](troubleshooting.md#escape-hatches) to work around the
  problems.

### Why?

- Support for `prepend` would force Sorbet to use more memory throughout an
  entire codebase, even if the codebase makes no use of `prepend`. Usage of
  `prepend` is far more rare than the cost it would inflict in terms of memory.

- Supporting `prepend` would add implementation complexity to Sorbet's
  internals. For example: consider how to do
  [Override Checking](override-checking.md) and
  [generic bounds checking](generics.md#bounds-on-type_members-and-type_templates-fixed-upper-lower)
  in the presence of prepended modules.

- Historically, Sorbet was developed at Stripe, which lints against usage of
  `prepend`.

- Historically and maybe still today: `sorbet-runtime` had (has?) poor support
  for runtime-checked type annotations on methods defined with prepended
  modules.

## Refinements and `refine do`

```ruby
class C
  def foo
    puts "C#foo"
  end
end

module M
  refine C do
    def foo
      puts "C#foo in M"
    end
  end
end

using M

c = C.new

c.foo # prints "C#foo in M"
```

Sorbet does not support refinements.

### Alternatives

- Use an RBI file to define the methods. Sorbet will assume that the methods are
  defined everywhere, not just where the `using` directive lives. This means
  that Sorbet will not reject code that would not have caused problems at
  runtime, at the expense of not catching situations that might have.

- Use an [Escape Hatch](troubleshooting.md#escape-hatches).

- Use a monkey patch.

### Why?

- While refinements are better than monkey patching, they still amount to monkey
  patching. Sorbet's role as a type checker is not only to catch errors, but to
  steer people towards simpler designs.

- Historically, Sorbet was developed at Stripe, which does not use refinements.

- Support for refinements would add implementation complexity to Sorbet.

- Supporting refinements would require doing program-wide work to discover and
  use refinements even if a codebase does not use them at all, which comes with
  a performance cost.

## Creating method aliases to methods in parent classes

```ruby
class Parent
  def defined_in_parent; end
end

class Child < Parent
  alias_method :defined_in_child, :defined_in_parent
  # Sorbet thinks this method doesn't exist ^
end
```

Sorbet does not support aliasing to a method defined in a parent class from a
child class.

### Alternative

1.  Use [RBI files](rbi.md) to define the methods that would be defined this
    way:

    ```ruby
    # -- foo.rb --
    class Parent
      def defined_in_parent; end
    end

    class Child < Parent
      T.unsafe(self).alias_method :defined_in_child, :defined_in_parent
    end

    # -- foo.rbi --
    class Child < Parent
      def defined_in_child; end
    end
    ```

2.  Use an [Escape Hatch](troubleshooting.md#escape-hatches) to silence errors
    at call sites.

### Why?

Due to original design decisions made in Sorbet's architecture, all methods are
defined before inheritance information is resolved.

There is nothing fundamental or performance sensitive which requires making this
choice (i.e., resolving ancestor information does not require knowing the set of
defined methods). But backing out this design decision at this point would
require more work than we currently believe the payoff is.

Because this is not a fundamental nor ideological limitation, it's possible this
feature may gain support in the future.

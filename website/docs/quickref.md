---
id: quickref
title: Quick Reference
---

> This guide is a quick reference for people already somewhat familiar with
> Sorbet. For getting started with Sorbet, see [Adopting Sorbet](adopting.md).

## Enabling type checking

To [enable static checking](static.md) with `srb`, add this line (called a
sigil) to the top of your Ruby file:

```ruby
# typed: true
```

This file will now be checked by Sorbet!

However, not much will really change. Sorbet knows about the types of methods in
the standard library, but not methods we've defined ourselves. To teach Sorbet
about the types of our methods, we have to add [signatures](sigs.md) above the
method:

```ruby
# typed: true

# (1) Bring T::Sig into scope:
require 'sorbet-runtime'

class Main
  # (2) extend T::Sig to get access to `sig` for annotations:
  extend T::Sig

  # (3) Add a `sig` annotation above your method:
  sig {params(x: String).returns(Integer)}
  def self.main(x)
    x.length
  end

  # alternatively, for a method with no parameters:
  sig {returns(Integer)}
  def no_params
    42
  end
end
```

Now `srb` **and** `sorbet-runtime` will check that our `Main.main` method is
given only `String`s and returns only `Integer`s:

- `srb` will do this with [static checks](static.md).
- `sorbet-runtime` will do this by wrapping `Main.main` with [dynamic
  checks](runtime.md) that run every time the method is called.


## Type System

The complete type system reference can be found to the left. Here's a quick
table of contents:

- [Integer](class-types.md), [String](class-types.md),
  [T::Boolean](class-types.md) – Class Types
- [T.nilable](nilable-types.md) – Nilable Types
- [T.any](union-types.md) – Union Types
- [T.let](type-assertions.md), [T.cast](type-assertions.md),
  [T.must](type-assertions.md), [T.assert_type!](type-assertions.md) – Type
  Assertions
- [[Type1, Type2]](tuples.md) – Tuple Types
- [{key1: Type1, key2: Type2}](shapes.md) – Shape Types
- [T.untyped](untyped.md)
- [T.noreturn](noreturn.md)
- [T.type_alias](type-aliases.md) – Type Aliases
- [T::Array](stdlib-generics.md), [T::Hash](stdlib-generics.md),
  [T::Set](stdlib-generics.md), [T::Enumerable](stdlib-generics.md) – Generics
  in the Standard Library
- [T.proc](procs.md) – Proc Types
- [T.class_of](class-of.md)
- [T.self_type](self-type.md)
- [T.all](intersection-types.md) – Intersection Types


## How do I...

### Figure out what's going on?

There are a number of common strategies that help when tracking down confusing
behavior in Sorbet:

[Troubleshooting](troubleshooting.md)

### Run Sorbet on a small example?

The easiest way to try out Sorbet on small examples is to use
[sorbet.run](https://sorbet.run), which runs `sorbet` (the static component) in
your browser.

[→ sorbet.run](https://sorbet.run)

### Run Sorbet on a small file?

Consider this file:

```ruby
# -- foo.rb --
# typed: true
require 'sorbet-runtime'

class Main
  extend T::Sig

  sig {void}
  def self.main
    puts 'Hello, world!'
  end
end

Main.main
```

To check it statically:

```plaintext
❯ srb tc foo.rb
```

> **Note**: If there's a `sorbet/config` file in the current directory, this
> command will potentially run on more than just this single file. Run `srb tc`
> from a different folder to check a single file in isolation.

To test out how the runtime checks work:

```plaintext
❯ bundle exec ruby foo.rb
```

> **Note** that `foo.rb` can only reference constants in the file and the
> standard library.

### Run Sorbet on a whole project?

Each Sorbet project should have a `sorbet/config` file at the root of the
project, which describes how to typecheck the project. If we are at the root of
our project, we can check the whole project with:

```plaintext
❯ srb
```

For more information, see [Adopting Sorbet](adopting.md).

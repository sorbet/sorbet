---
id: quickref
title: Quick Reference
---

> This guide is a quick reference for people already somewhat familiar with
> Sorbet. For getting started with Sorbet, see [Adopting Sorbet](adopting.md).

## Enabling type checking

To [enable static checking](static.md) with `sorbet`, add this line (called a
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

Now `sorbet` **and** `sorbet-runtime` will check that our `Main.main` method is
given only `String`s and returns only `Integer`s:

- `sorbet` will do this with [static checks](static.md).
- `sorbet-runtime` will do this by wrapping `Main.main` with [dynamic
  checks](runtime.md) that run every time the method is called.


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

```
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

To run `sorbet` on it:

```
❯ sorbet foo.rb
```

To test out how `sorbet-runtime` works:

```
❯ bundle exec ruby foo.rb
```

> **Note** that `foo.rb` can only reference constants in the file and the
> standard library.

### Run Sorbet on a whole project?

Each Sorbet project should have a [file list](adopting.md). If this file is
named `FILE_LIST`, we can run:

```
❯ sorbet @FILE_LIST
```

To generate a rudimentary file list:

```
❯ find . -name '*.rb' -o -name '*.rbi' > FILE_LIST
```


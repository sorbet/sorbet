---
id: strict
title: Strict Mode
---

> TODO(jez) This page is still a fragment. Contributions welcome!

As introduced in [Enabling Static Checks](static.md), Sorbet has multiple
strictness modes. The default is `# typed: false`, which silences all type
errors. Next up is `# typed: true`, where type errors are reported. There's also
`# typed: strict`, which is a way to require that everything which needs an
annotation gets an annotation.

Specifically, in a `# typed: strict` file it's an error to omit type annotations
for:

- methods
- instance variables
- class variables
- constants

> Why is this not an error in `# typed: true`? In that strictness level,
> unannotated methods, instance variables, and constants are assumed to be
> `T.untyped`.

## Annotating Constants

Sorbet does not, by default, infer the types of constants, but you can specify
them using T.let:

```ruby
NAMES = T.let(["Nelson", "Dmitry", "Paul"], T::Array[String])
```

## Declaring Class and Instance Variables

Declare an instance variable using `T.let` in a class's constructor:

```ruby
class MyObj
  def initialize
    @foo = T.let(0, Integer)
  end
end
```

We can also declare class variables and instance variables on a singleton class
using `T.let` outside of any method:

```ruby
class HasVariables
  # Class variable
  @@llamas = T.let([], T::Array[Llama])

  # Instance variable on the singleton class
  @alpaca_count = T.let(0, Integer)
end
```


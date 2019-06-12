---
id: type-annotations
title: Type Annotations
---

Sorbet provides the most value when it has a wealth of static type information
to work with, which in turn requires the programmer to add annotations that
inform Sorbet of the intended static types of elements of your program. Sorbet
also allows the programmer to opt-in to greater levels of static type rigor: at
lower [strictness modes](static.md), Sorbet will allow some definitions by
untyped, but at `# typed: strict`, Sorbet requires that everything which can
have a type annotation _must_ have one.

Specifically, in a `# typed: strict` file it's an error to omit type annotations
for:

- methods
- instance variables
- class variables
- constants

> Why is this not an error in `# typed: true`? In that strictness level,
> unannotated methods, instance variables, and constants are assumed to be
> `T.untyped`. This allows a programmer to benefit from static type-checking
> while also permitting untyped or partially-untyped definitions at the same
> time.

Type annotations for methods are provided using a `sig` before the method
definition. Method type annotations are
[described in great detail on the Method Signatures](sigs.md). Other type
annotations are usually provided using the `T.let`
[type assertion](type-assertions.md).

## Annotating Constants

Sorbet does not, by default, infer the types of constants, but you can specify
them using `T.let`:

```ruby
NAMES = T.let(["Nelson", "Dmitry", "Paul"], T::Array[String])
```

## Declaring Class and Instance Variables

To declare the static type of an instance variable, we can use `T.let` in a
class's constructor:

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

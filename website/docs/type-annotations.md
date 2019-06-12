---
id: type-annotations
title: Type Annotations
---

Sorbet provides the most value when it has a wealth of programmer-supplied
static types. However, because Sorbet implements a
[gradual type system](gradual.md), it treats most definitions without explicit
annotations as [untyped](untyped.md). This means that Sorbet can only use static
types for methods, constants, instance variables, and class variables if they
are accompanied with explicit static types.

> What about local variables? Sorbet _does_ do limited type inference for local
> variables based on how they are initialized, so type assertions may not be
> necessary for local variables. However, they may still be useful, especially
> if you want a local variable to have a more general type than the one that
> Sorbet infers.

Sorbet also allows the programmer to opt-in to greater levels of static type
rigor: at lower [strictness modes](static.md), Sorbet will allow some
definitions to be untyped, but at `# typed: strict`, Sorbet requires obligatory
type annotations on anything which can be annotated. Specifically, in a
`# typed: strict` file it's an error to omit type annotations for:

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

## Annotating constants

Sorbet does not, by default, infer the types of constants, but you can specify
them using `T.let`:

```ruby
NAMES = T.let(["Nelson", "Dmitry", "Paul"], T::Array[String])
```

## Declaring class and instance variables

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

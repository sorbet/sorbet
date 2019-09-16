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

The exception to the above list is that Sorbet does not need type annotations
for local variables. In the absence of a type annotation, Sorbet will infer the
type of a local variable based on how the variable is initialized. It is still
possible to provide type annotations for local variables.

Type annotations for methods are provided using a `sig` before the method
definition. Method type annotations are
[described in great detail on the Method Signatures](sigs.md). Other type
annotations are provided using the `T.let` [type assertion](type-assertions.md).

## Annotating constants

Sorbet does not, by default, infer the types of constants, but they can be
specified using `T.let`:

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
using `T.let` at the top-level of a class:

```ruby
class HasVariables
  # Class variable
  @@llamas = T.let([], T::Array[Llama])

  # Instance variable on the singleton class
  @alpaca_count = T.let(0, Integer)
end
```

Sorbet requires that instance and class variables are defined in these specific
places to guarantee that they're initialized. But sometimes requiring that these
variables be declared in specific places is too restrictive. Sorbet allows an
instance variable to be declared **anywhere** so long as the type is at least
nilable:

```ruby
class A
  def foo
    # Does NOT have to be declared in `initialize`, because it's nilable:
    @x = T.let(0, T.nilable(Integer))
  end

  def self.bar
    # Also works for `self.` methods:
    @y = T.let('', T.nilable(String))
  end
end
```

It's common to use this technique to add type annotations for instance variables
in functions that memoize their result:

```ruby
sig {returns(String)}
def current_user
  @user = T.let(@user, T.nilable(String))
  @user ||= ENV.fetch('USER')
end
```

## Why do I need to repeat types from the constructor?

A current shortcoming of Sorbet is it cannot reuse static type knowledge in
order to automatically determine the type of an instance or class variable. In
the following example, despite the fact that Sorbet knows that `x` has type
`Integer`, it still treats `@x` as `T.untyped` without an explicit type
annotation:

```ruby
class Foo
  sig {params(x: Integer, y: Integer).void}
  def initialize(x, y)
    @x = x
    @y = T.let(y, Integer)

    T.reveal_type(@x)  # T.untyped
    T.reveal_type(@y)  # Integer
  end
end
```

This is a known limitation of Sorbet, and we're considering ways to make it less
verbose in the future.

## Type annotations and strictness levels

Sorbet allows the programmer to opt-in to greater levels of static type rigor.
At lower [strictness modes](static.md), Sorbet allows definitions to be untyped,
but at `# typed: strict`, Sorbet requires explicit type annotations on any
definitions where it would have assumed `T.untyped` without an annotation
before. Specifically, in a `# typed: strict` file it's an error to omit type
annotations for:

- methods
- instance variables
- class variables
- constants

It may seem counterintuitive that Sorbet does _not_ require type annotations in
a file marked `# typed: true`, but this is an intentional part of Sorbet's
implementation of [gradual typing](gradual.md). In the `# typed: true`
strictness level, unannotated methods, instance variables, and constants are
assumed to be `T.untyped`. This allows a programmer to write untyped or
partially-typed definitions while still benefiting from type checking when
static type information is present.

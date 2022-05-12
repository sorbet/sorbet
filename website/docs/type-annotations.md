---
id: type-annotations
title: Declaring types for non-methods
sidebar_label: Type Annotations (non-sig)
---

Sorbet provides the most value when it has a wealth of programmer-supplied
static types.

However, because Sorbet implements a [gradual type system](gradual.md), it
treats most definitions without explicit annotations as [untyped](untyped.md).
This means that Sorbet can only use static types for methods, constants,
instance variables, and class variables if they are accompanied with explicit
static types.

For more information on why type annotations are required in Sorbet, see
[Why does Sorbet sometimes need type annotations?](why-type-annotations.md).

Type annotations for methods are provided using a `sig` before the method
definition. Method type annotations are
[described in great detail on the Method Signatures](sigs.md).

Other type annotations are provided using the `T.let`
[type assertion](type-assertions.md).

## Annotating local variables

Sorbet does not usually need type annotations for local variables, as it can
infer the type of the local variable based on how it is initialized. For
example, in the following program, Sorbet can tell `x` is an `Integer` based on
the fact that it is initialized with an expression that evaluates to an
`Integer`:

```ruby
x = 2 + 3
```

You may still provide a wider type annotation if you would like. This can
occasionally be helpful if you want the type of a variable to be broader than
Sorbet's inferred type, such as in situations where you are changing the value
of a variable in a loop to something that is broader than the expression that
you use to initialize the variable:

```ruby
# without this T.let, x would have the inferred type NilClass
x = T.let(nil, T.nilable(Integer))
(0..10).each do |n|
  x = n if n % 3 == 0
end
```

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
in functions that memoize their results by lazily initializing instance
variables:

```ruby
module B
  sig {returns(String)}
  def current_user
    unless defined?(@user)
      @user = T.let(ENV.fetch('USER'), T.nilable(String))
    end
    T.must(@user)
  end
end
```

## Limitations on instance variable inference

A current shortcoming of Sorbet is that in many cases it cannot reuse static
type knowledge in order to automatically determine the type of an instance or
class variable. In the following example, Sorbet will naturally understand that
`@x` is of type `Integer`, but it cannot determine the static type of `@y`
without a `T.let` and therefore treats it as `T.untyped`:

```ruby
class Foo
  sig {params(x: Integer, y: Integer).void}
  def initialize(x, y)
    @x = x
    @y = y + 0

    T.reveal_type(@x)  # Integer
    T.reveal_type(@y)  # T.untyped
  end
end
```

Sorbet can only infer the types of instance variables in a relatively specific
context: in particular, only when that instance variable is initialized to the
exact value of a parameter to the constructor in the body of the constructor. In
other cases, you will need to use `T.let` to explicitly give the types of
instance variables.

> **Note**: This particular limitation is because of how Sorbet performs
> typechecking: it needs to know the types of instance variables _before_ it
> typechecks method definitions, but even a simple expression like `y + 0` will
> require a typechecking pass to determine what its resulting type is, which
> means Sorbet won't be able to tell what type to infer for an instance variable
> until _after_ it has already started typechecking the method where instance
> variables are defined. Sorbet resolves this cyclical dependency by either
> using types that you have explicitly defined with `T.let`, or by relying on
> simple code patterns like the one described above.

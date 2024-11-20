---
id: sigs
title: Method Signatures
sidebar_label: sig
---

> This page describes the syntax of method signatures, or `sig`s. For a complete
> reference of the types available for use within a `sig`, see the "Type System"
> section to the left.

Method signatures are the primary way that we enable static and dynamic type
checking in our code. In this document, we'll answer:

- How to add signatures to methods.
- Why we'd want to add signatures in the first place.

Signatures are valid Ruby syntax. To be able to write signatures, we first
`extend T::Sig` at the top of our class or module:

```ruby
extend T::Sig
```

## `sig`: Annotating method signatures

The basic syntax looks like this:

```ruby
sig {params(x: SomeType, y: SomeOtherType).returns(MyReturnType)}
def foo(x, y); ...; end
```

It's also possible to break a `sig` up across multiple lines. Here's the same
signature as above, rearranged:

```ruby
sig do
  params(
    x: SomeType,
    y: SomeOtherType,
  )
  .returns(MyReturnType)
end
def foo(x, y); ...; end
```

In every signature, there is an optional `params` section, and a required
`returns` section. Here's a complete example:

```ruby
# typed: true
require 'sorbet-runtime'

class Main
  # Bring the `sig` method into scope
  extend T::Sig

  sig {params(x: String).returns(Integer)}
  def self.main(x)
    x.length
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Arequire%20'sorbet-runtime'%0A%0Aclass%20Main%0A%20%20%23%20Bring%20the%20%60sig%60%20method%20into%20scope%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20String).returns(Integer)%7D%0A%20%20def%20self.main(x)%0A%20%20%20%20x.length%0A%20%20end%0Aend">→
View on sorbet.run</a>

## `params`: Annotating parameter types

In the `sig` we refer to all parameters **by their name**, regardless of whether
it's a positional, keyword, block, or rest parameter. Once we've annotated the
method, Sorbet will automatically infer the types of any local variables we use
in the method body.

### Positional parameters

Here's the syntax for required and optional **positional** parameters:

```ruby
sig do
  params(
    x: String,           # required positional param
    y: String,           # optional positional param
    z: T.nilable(String) # optional *AND* nilable param
  )
  .returns(String)
end
def self.main(x, y = 'foo', z = nil)
  x + y + (z ? z : '')
end
```

### Keyword parameters (kwargs)

Here's the syntax for required and optional **keyword** parameters:

```ruby
sig do
  params(
    x: String,            # required keyword param
    y: String,            # optional keyword param
    z: T.nilable(String)  # optional *AND* nilable keyword param
  )
  .void
end
def self.main(x:, y: 'foo', z: nil)
  # ...
end
```

### Rest parameters

Sometimes called splats. There are two kinds of rest parameters: "all the
arguments" (`*args`) and "all the keyword arguments" (`**kwargs`):

Types for rest parameters frequently trip people up. There's a difference
between what's written in the sig annotation and what type that variable has in
the method body:

```ruby
sig do
  params(
    # Integer describes a single element of args
    args: Integer, # rest positional params
    # Float describes a single value of kwargs
    kwargs: Float  # rest keyword params
  )
  .void
end
def self.main(*args, **kwargs)
  # Positional rest args become an Array in the method body:
  T.reveal_type(args) # => Revealed type: `T::Array[Integer]`

  # Keyword rest args become a Hash in the method body:
  T.reveal_type(kwargs) # => Revealed: type `T::Hash[Symbol, Float]`
end
```

Notice that in the sig, `args` is declared as `Integer`, but in the method body
Sorbet knows that `args` is actually a `T::Array[Integer]` because it can see
from the method definition that `args` is a rest parameter.

It's similar for `kwargs`: it's declared as `Float`, but in the method body
Sorbet knows that it'll be a Hash from `Symbol` keys to `Float` values.

> **Note**: The choice to use this syntax for annotating rest parameters in
> Sorbet was informed by precedent in other languages (most notably Scala).

### Block parameters

```ruby
sig do
  params(
    blk: T.proc.returns(NilClass)
  )
  .void
end
def self.main(&blk)
  # ...
end
```

See [Blocks, Procs and Lambda Types](procs.md) for more information on how to
write type annotations for a method's block parameter.

### No parameters

When a method has no parameters, omit the `params` from the `sig`:

```ruby
sig {returns(Integer)}
def self.main
  42
end
```

See the next section for more information.

## `returns` & `void`: Annotating return types

Unlike `params`, we _have_ to tell Sorbet what our method returns, even if it
has "no useful return." For example, consider this method:

```ruby
def main
  5.times do
    puts 'Hello, world!'
  end
end
```

We care more about what **effect** this method has (printing to the screen) than
what this method **returns** (`5`). We _could_ write a `sig` like this:

```ruby
sig {returns(Integer)}   # ← Problematic! Read why below...
```

This is annoying for a bunch of reasons:

- We'd get a useless type error if someone added `puts 'Goodbye, world!'` at the
  bottom of `main`. Instead of returning `5` (`Integer`), the method would now
  return `nil` (`NilClass`).

- Call sites in untyped code can implicitly depend on us always returning an
  `Integer`. For example, what if people think returning `5` is actually some
  sort of exit code?

Instead, Sorbet has a special way to mark methods where we only care about the
effect: `void`:

```ruby
sig {void}
```

Using `void` instead of `returns(...)` does a number of things:

- Statically, `srb` will let us return any value (for example, returning either
  `5` or `nil` is valid).

- Also statically, `srb` will error when typed code tries to inspect the result
  of a `void` method.

- In the runtime, `sorbet-runtime` will _throw away_ the result of our method,
  and return a dummy value instead. (All `void` methods return the same dummy
  value.)

  If you do not want this behavior, either use `returns(T.anything)` instead
  ([docs for `T.anything`](anything.md)), or
  [disable runtime checking](runtime.md) for that method (or all methods).

Replacing the return value with a meaningless value prevents untyped code from
silently depending on what a typed method returns, so that the implementation is
free to change without worry of breaking existing code which silently depended
on the result of the method being meaningful.

Concretely, here's a full example of how to use `void` to type methods with
useless returns:

```ruby
# typed: true
require 'sorbet-runtime'

class Main
  extend T::Sig

  # (1) greet has a useless return:
  sig {params(name: String).void}
  def self.greet(name)
    puts "Hello, #{name}!"
  end

  # (2) name_length must be given a string:
  sig {params(name: String).returns(Integer)}
  def self.name_length(name)
    name.length
  end
end

# (3) It's an error to pass a void result to name_length:
Main.name_length(Main.greet('Alice')) # => error!
```

## Adding sigs to class methods

There are many ways to define class (static) methods in Ruby. How a method is
defined changes where the `extend T::Sig` line needs to go. These are the two
preferred ways to define class methods with sigs:

1.  `def self.greet`

    ```ruby
    class Main
      # In this style, at the top level of the class
      extend T::Sig

      sig {params(name: String).void}
      def self.greet(name)
        puts "Hello, #{name}!"
      end
    end
    ```

2.  `class << self`

    ```ruby
    class Main
      class << self
        # In this style, inside the `class << self`
        extend T::Sig

        sig {params(name: String).void}
        def greet(name)
          # ...
        end
      end
    end
    ```

## Why do we need signatures?

Taking a step back, why do we need `sig`s in the first place?

Sorbet does type inference for local variables within methods, and then requires
annotations for method parameters and return types. This mix of type inference
and type annotations balances being **explicit** with being **powerful**:

- With a small amount of information, Sorbet can power **autocompletion
  results** and catch **type errors**.
- Since there's no type inference across methods, each method can be typechecked
  **100% in parallel**, for fast performance. Other people can't write code
  which makes typechecking _your_ code slow.
- Method signatures serve as machine-checked documentation for whoever reads the
  code.

So basically: the complexity of Ruby requires it, it enables Sorbet to be
performant, and it encourages better development practices. Anecdotally, we've
seen all three of these things have a positive effect on development.

## Why are signatures Ruby syntax?

For example, Sorbet could have re-used YARD annotations, or extended Ruby with
new syntax.

There are a number of reasons why we have type annotations as valid Ruby method
calls:

- The existing ecosystem of Ruby tooling still works.

  Editor syntax highlighting, Ruby parsers, RuboCop, IDEs, and text editors, and
  more all work out of the box with Sorbet's type annotations.

- No runtime changes required.

  If Sorbet introduced new syntax, type-annotated code would no longer be
  directly runnable simply with `ruby` at the command line. This means no build
  step is required, and no special changes to the core language.

- Runtime checking is a feature.

  In a [gradual type system](gradual.md) like Sorbet, the static checks can be
  turned off at any time. Having [runtime-validated](runtime.md) type
  annotations gives greater confidence in the predictions that `srb`
  [makes statically](static.md).

- Type assertions in code would be inevitable.

  Having constructs like [`T.let` and `T.cast`](type-assertions.md) work in line
  requires that type annotations already be syntactically valid Ruby (having
  `T.let` and `T.cast` to do type refinements and assertions are central to
  Sorbet being a gradual type system). Since types must already be valid Ruby,
  it makes sense to have `sig`s be valid Ruby too.

## Can I skip writing `extend T::Sig` everywhere?

To skip writing `extend T::Sig` inside every class that wants to use `sig`, use
this monkey patch:

```ruby
class Module
  include T::Sig
end
```

Since every singleton class descends from `Module`, this will make the `sig`
method available in every class body.

This involves a monkey patch, and is **not** required to use Sorbet. But the
most earnest users of Sorbet all eventually add this monkey patch, because
`extend T::Sig` ends up getting written into almost every class.

We recommend putting this monkeypatch in the same file that

- requires `sorbet-runtime`, and
- sets up any [`T::Configuration`](tconfiguration.md) `sorbet-runtime`
  configurations

So that it's impossible to get one of these three things without the others.

The upside:

- Drops the "activation energy" required to add the first `sig` to a class.
  Simply start writing `sig` above the current method.

- Fewer lines dedicated to type annotations.

The downside:

- Adds a monkey patch to a Ruby standard library class, which might conflict
  with other methods defined by the current project or its gems.

- If users have _only_ required `sorbet-runtime` and forgotten to require the
  file defining this monkey patch, the code can fail at runtime in unexpected
  ways.

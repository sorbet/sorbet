---
id: sigs
title: Method Signatures
sidebar_label: Signatures
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
```

In every signature, there is an optional `params` section, and a required
`returns` section.

### Example of Complete File

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

## `params`: Annotating parameter types

In the `sig` we refer to all parameters (aka. arguments) **by their name**,
regardless of whether it's a positional, keyword, block, or rest parameter. Once
we've annotated the method, Sorbet will automatically infer the types of any
local variables we use in the method body.

### Positional Parameters

```ruby
sig do
  params(
    x: String, # required positional param
    y: String  # optional positional param
  )
  .returns(String)
end
def self.main(x, y = 'foo')
  x + y
end
```

### Keyword Parameters (kwargs)

```ruby
sig do
  params(
    x: String,            # required keyword param
    y: T.nilable(String)  # optional keyword param
  )
  .void
end
def self.main(x:, y: nil)
  # ...
end
```

### Rest Parameters

Sometimes called "splat" arguments.

```ruby
sig do
  params(
    # Rest params will be arrays. Write the type of their elements.
    # E.g. for arrays of String, we write String.
    x: String, # rest positional params
    y: String  # rest keyword params
  )
  .returns(String)
end
def self.main(*x, **y)
  # Rest args become an Array in the method body:
  T.reveal_type(x) # => Revealed type: `T::Array[String]`

  x.map(&:to_s).concat(y.values).join(',')
end
main(91, 92, a: 'foo', b: 'bar') # => "91,92,foo,bar"
```

### Block Parameter

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

### No Parameters

When a method has no parameters, omit the `params` from the `sig`:

```ruby
sig {returns(Integer)}
def self.main
  42
end
```

### Local Variable Types can be inferred from Parameter Types

```ruby
sig do
  params(
    x: String,
    y: String
  )
  .returns(Integer)
end
def self.main(x, y)
  # Sorbet infers (!) the type of a:
  a = x.length + y.length

  # We can use `T.reveal_type` to ask Sorbet for the type of an expression:
  T.reveal_type(a) # => Revealed type: `Integer`
end
```

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
  value.) This prevents untyped code from silently depending on what we return.

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

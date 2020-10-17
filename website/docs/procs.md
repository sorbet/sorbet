---
id: procs
title: Blocks, Procs and Lambda Types
sidebar_label: Blocks, Procs, & Lambdas
---

```ruby
T.proc.params(arg0: Arg0Type, arg1: Arg2Type, ...).returns(ReturnType)
```

This is the type of a `Proc` (such as a block passed to a method as a `&blk`
parameter) that accepts arguments of types `Arg0Type`, `Arg1Type`, etc., and
returns `ReturnType`.

At present, all parameters are assumed to be required positional parameters. We
may add support for optional or keyword parameters in the future, if there is
demand.

Types of procs are not checked at all at runtime (whereas methods are), and
serve only as hints to `srb` statically (and for documentation).

Here's a larger example:

```ruby
# typed: true

# (1) Declare the type of a block arg with `T.proc`:
sig {params(blk: T.proc.params(arg0: Integer).void).void}
def foo(&blk)
  x = T.let('not an int', T.untyped)

  # (2) The `T.proc` annotation is not checked at runtime.
  #     (This won't raise even though x is not an Integer)
  yield x
end

# (3) Sorbet incorporates the `T.proc` annotation into what it knows statically
foo do |x|
  T.reveal_type(x) # Revealed type: Integer
end
```

## Optional blocks

Use `T.nilable` to declare that a method can take an optional block:

```ruby
# typed: true

extend T::Sig

# (1) Declare optional block with `T.nilable(...)`:
sig {params(blk: T.nilable(T.proc.void)).void}
def foo(&blk)
  T.reveal_type(blk)   # Revealed type: `T.nilable(T.proc.void)`

  # (2) Use `block_given?` to check whether a method was given a block:
  if block_given?
    T.reveal_type(blk) # Revealed type: `T.proc.void`
  end

  # (3) Equivalently:
  if blk
    T.reveal_type(blk) # Revealed type: `T.proc.void`
  end
end
```

Because we used `block_given?` above, Sorbet will know that outside the `if`
expression `blk` might be `nil`, while inside the `if` it's not `nil`.

## Annotating methods that use `yield`

Ruby's `yield` keyword yields control to the provided block even when a method
declaration doesn't mention a block parameter:

```ruby
def foo
  yield
end
```

This is valid Ruby, and Sorbet will accept this code too. Implicitly, Sorbet
will know that the method might accept a block, but it will treat the block
itself as `T.untyped`. In order to give a signature to this method, the block
parameter will need a name:

```ruby
def foo(&blk) # <-
  yield
end
```

And once it has a name, the method can be given a sig:

```ruby
sig {params(blk: T.proc.returns(Integer)).returns(Integer)}
def foo(&blk) # <-
  yield
end
```

Note that the `yield` itself in the method body doesn't need to change at all.
Since every Ruby method can only accept one block, both Ruby and Sorbet are able
to connect the `yield` call to the `blk` parameter automatically.

## Modifying the self type

Many Ruby constructs accept a block argument in one context, but then execute it
in a different context entirely. This means that the methods that exist inside
the block are not the methods that exist outside the block (like is usually the
case).

As an example, this is how Sorbet's `sig` pattern works:

```ruby
# (1) `params` doesn't exist outside the `sig` block:
params(x: Integer).void # error: Method `params` does not exist

# (2) But `params` does exist inside the `sig` block:
sig do
  params(x: Integer).void # ok!
end
```

This also happens a lot with certain Rails APIs, etc. Sorbet has direct support
for this sort of pattern using an extra argument on `T.proc` called `.bind`:

```ruby
# (0) We're simplifying how `sig` specifically works a bit here,
#     but the general ideas apply.
module T::Sig
  # (1) Use `T.proc.bind` to annotate the context in which the block will run
  sig {params(blk: T.proc.bind(T::Private::Methods::DeclBuilder).void).void}
  def sig(&blk); end
end

# This comes from a private, internal sorbet-runtime API that implements sigs:
module T::Private::Method::DeclBuilder
  def params(params); end
  def returns(type); end
  def void; end
end
```

Here's another example that's a little more contrived but which shows both the
method call site and method definition site:

```ruby
# typed: true
extend T::Sig

# (1) Use `.bind(String)` to say "will be run in the context of a String":
sig {params(blk: T.proc.bind(String).returns(String)).returns(String)}
def foo(&blk)
  "hello".instance_eval(&blk)
end

upcased = foo do
  # (2) Sorbet knows that `self.upcase` is available because of the `.bind`
  self.upcase
end

puts(upcased) # => "HELLO"
```

## Proc.new vs proc

In Ruby there's several ways to create a Proc: `proc` and `Proc.new`. Sorbet
handles them differently because of how parameter arity works internally.
Basically, you can use `T.proc` as the type for a `proc` (or `lambda {}` or
`-> {}`), but if you create your Proc using `Proc.new`, then the type is a
`Proc`.

The main downside of the `Proc.new` approach is that you can't set argument
types on it.

Here's an example:

```ruby
sig {returns(T.proc.params(a1: Integer).returns(Integer))}
def lowercase_proc
  # if you are using `proc`, use `T.proc` in your sig
  # this is the preferred approach
  proc {|n| n * 2 }
end

sig {returns(Proc)}
def proc_dot_new
  # if you are using `Proc.new`, use `Proc` in your sig
  # note that you won't be able to define arguments/return types in your sig
  # avoid this approach if possible
  Proc.new {|n| n * 2 }
end
```

In general, you're better off avoiding `Proc.new` if you can. There's a
[rubocop rule](https://docs.rubocop.org/rubocop/0.92/cops_style.html#styleproc)
you can use to enforce this.

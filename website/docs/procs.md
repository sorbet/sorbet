---
id: procs
title: Proc Types
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
T.proc.params(arg0: Arg0Type, arg1: Arg2Type, ...).returns(ReturnType)
```

This is the type of a `Proc` (such as a block passed to a method as a `&blk`
parameter) that accepts arguments of types `Arg0Type`, `Arg1Type`, etc., and
returns `ReturnType`.

At present, all parameters are assumed to be required positional parameters. We
may add support for optional or keyword parameters in the future, if there is
demand.

Types of procs are not checked at all at runtime (the same way methods are), and
serve only as hints to `srb` statically (and for documentation).

## Annotating Block Parameters With Yield

Ruby's `yield` keyword can invoke a block without requiring a block argument in
the parameter list. Even so, it's a good idea when using Sorbet to name those
block parameters anyway, because without a local name there's no way to give the
block parameter a static type annotation. In fact, at the typed level
`# typed: strict`, Sorbet will require that all block parameters have names
_even if the methods use_ `yield`. For example, Sorbet will **reject** the
following code snippet because the method uses `yield` but has not named its
block parameter, which in turn means that it lacks a declared type in the `sig`:

```ruby
# typed: strict
sig { void }
def call_twice  # error: Method call_twice uses yield but does not mention a block parameter
  yield
  yield
end
```

This error can be fixed without modifying the method body to use `block.call`:
It's possible to give the method a named block parameter like `&blk`, and
`yield` will still work exactly as it did, invoking the block regardless of
whether or not it was named. Once the block parameter has a name, it's in turn
possible to associate that name with a type:

```ruby
# typed: strict
sig { params(blk: T.proc.void).void }
def call_twice(&blk)
  yield
  yield
end
```

## Modifying The Self Type

While blocks are usually evaluated in the context in which they appear, it's
possible to use the methods `instance_exec` and `instance_eval` to evaluate them
in some other specified context, which changes the meaning of `self` in the body
of that block. To convey this to Sorbet, we add a `.bind(...)` clause on the
`T.proc` type that indicates the context in which the method will be invoked:

```ruby
# typed: true
extend T::Sig

sig { params(blk: T.proc.bind(String).returns(String)).returns(String) }
def foo(&blk)
  "Hello".instance_eval(&blk)
end

puts foo { self.upcase }
```

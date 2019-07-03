---
id: troubleshooting
title: Troubleshooting
---

This is one of three docs aimed at helping answer common questions about Sorbet:

1.  [Troubleshooting](troubleshooting.md) (this doc)
1.  [Frequently Asked Questions](faq.md)
1.  [Sorbet Error Reference](error-reference.md)

This doc covers two main topics:

- When stuck, how to [find out **why**](#validating-our-assumptions).
- Regardless of why, [how to **get unstuck**](#escape-hatches).

## Validating our assumptions

When faced with a type error, checking our assumptions is step number one. The
first question to ask is:

> Are my files `# typed: false` or `# typed: true`?

There are also some tools for helping debug type-related errors:

### `T.reveal_type`

If we wrap a variable or method call in `T.reveal_type`, Sorbet will show us
what type it thinks that variable has. This is a super powerful debugging
technique! `T.reveal_type` should be **one of the first tools** to reach for
when debugging a confusing error.

Try making a hypothesis of what the problem is ("I think the problem is...") and
then create small examples in <https://sorbet.run> to test the hypothesis with
`T.reveal_type`. For example:

```ruby
# typed: true
extend T::Sig

sig {params(xs: T::Array[Integer]).returns(Integer)}
def foo(xs)
  T.reveal_type(xs.first) # => Revealed type: `T.nilable(Integer)`
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(xs%3A%20T%3A%3AArray%5BInteger%5D).returns(Integer)%7D%0Adef%20foo(xs)%0A%20%20T.reveal_type(xs.first)%20%23%20%3D%3E%20Revealed%20type%3A%20%60T.nilable(Integer)%60%0Aend">
  → View on sorbet.run
</a>

With this example we see that `xs.first` returns `T.nilable(Integer)`.

Frequently when troubleshooting type errors, either something is `nil` or
`T.untyped` unexpectedly. We can use `T.reveal_type` to track down where the
type originated.

> **Remember**: Sorbet is a [gradual type system](gradual.md). Even if most code
> in a codebase is typed, it's still possible for code to be untyped!

### sorbet.run

Sorbet is available in an online sandbox:

[→ sorbet.run](https://sorbet.run)

The online version of Sorbet can typecheck...

- core Ruby language constructs (like control flow)
- Ruby standard libraries (like `Array` and `Hash`)
- **select** DSLs (like `T::Struct` `prop`s)

Using [sorbet.run](https://sorbet.run) to make a minimal repro is a great way to
isolate whether something is "just how Sorbet works" or is acting strangely in
conjunction with the code in a specific project locally.

> **Note**: sorbet.run only shows how the static component of Sorbet works. To
> test the runtime component, see the [Quick Reference](quickref.md).

### Run the code

Types predict values. Is Sorbet's prediction accurate? Try using a Ruby REPL
(like `irb` or `binding.pry`) to run code. Does it actually work? If it doesn't
actually work, Sorbet caught a bug!

Otherwise, there are a handful of reasons why Sorbet predicts code will not work
even when it does:

- Maybe the code ran fine for all provided input values, but there's an edge
  case being left untested.

- Maybe the code uses a method from the standard library that is missing or
  typed incorrectly (our standard library shims are sometimes incomplete).

- Maybe a method Sorbet thinks doesn't exist actually **does** exist because it
  was dynamically defined with `define_method` or `missing_method`. See
  [Escape Hatches](#escape-hatches) below for working around this.

### `*.rbi` files & missing methods

One of the most confusing errors from Sorbet can be "7003: Method does not
exist." Ruby is a very dynamic language, and methods can be defined in ways
Sorbet cannot see statically. It's possible that even though a method exists at
runtime, Sorbet cannot see it.

However, we can use `*.rbi` files to declare methods to Sorbet so that it
**can** see them statically. For more information about RBI files:

[→ RBI files](rbi.md)

### Help with common errors

The tips above are very generic and apply to lots of cases. For some common
gotchas when using Sorbet, here are two more specific resources:

- [Sorbet Error Reference](error-reference.md)

  This document is a collaborative reference with suggestions for commonly
  encountered error codes. You should see links to this document in Sorbet's
  error output.

- [FAQ](faq.md)

  This is a list of questions people commonly have when working with Sorbet and
  the runtime type system. Skim it to see if it says anything useful!

## Escape Hatches

Regardless of whether we can figure out the root cause of the error at hand,
Sorbet is designed as a [gradual type system](gradual.md). This means there will
always be **escape hatches** to silence the problem.

### `T.unsafe`

By wrapping an expression like `x` in `T.unsafe(...)` we can ask Sorbet to
forget the result type of something.

One case when this is useful is when we know for sure that a method exists, but
Sorbet doesn't know that method exists statically:

```ruby
# typed: true
class A
  def method_missing(method)
    puts "Called #{method}"
  end
end

A.new.foo # error: Method `foo` does not exist on `A`
```

In cases like this, we have a couple options. The first one is just: rewrite the
code. Code that's hard for Sorbet to understand is frequently hard for
developers to understand. By rewriting confusing code, we benefit all future
readers.

However, in the case when we're **sure** that we don't want to refactor the
code, we have an escape hatch: `T.unsafe`. It looks like this:

```ruby
# typed: true
class A
  def method_missing(method)
    puts "Called #{method}"
  end
end

T.unsafe(A.new).foo # => ok
```

The `T.unsafe` call effectively says to Sorbet, "trust me, I know what I'm
doing." At this point, the burden of correctness shifts from Sorbet to the
programmer.

### `T.unsafe(self)`

Note that the call to `T.unsafe` must wrap the [receiver] of the method call. In
this example:

[receiver]:
  https://stackoverflow.com/questions/916572/in-ruby-what-does-the-receiver-refer-to

```ruby
# typed: true
class A
  def method_missing(method)
    puts "Called #{method}"
  end

  def initialize
    foo # error: Method `foo` does not exist on `A`
  end
end
```

`foo` is a method that's being called without on the implicit receiver of
`self`. Another way of saying that is that `foo` is the same as `self.foo` in
Ruby. So to use unsafe to silence this error, we have to make the `self.foo`
explicit:

```ruby
# typed: true
class A
  def method_missing(method)
    puts "Called #{method}"
  end

  def initialize
    T.unsafe(self).foo # ok
  end
end
```

## Alternatives to `T.unsafe`

`T.unsafe` is maximally unsafe. It forces Sorbet to forget all type information
statically---sometimes this is more power than we need. For the cases where we
the programmer know of an invariant that isn't currently expressed in the type
system, [`T.cast`](type-assertions#tcast) is a good middle-ground.

<!-- TODO(jez) Document .on_failure / .checked once API is stable. -->

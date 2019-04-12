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

If we wrap a variable or method call in `T.reveal_type`, Sorbet will show
us what type it thinks that variable has. This is a super powerful debugging
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
[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(xs%3A%20T%3A%3AArray%5BInteger%5D).returns(Integer)%7D%0Adef%20foo(xs)%0A%20%20T.reveal_type(xs.first)%20%23%20%3D%3E%20Revealed%20type%3A%20%60T.nilable(Integer)%60%0Aend)

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
  was dynamically defined with `define_method` or `missing_method`. See [Escape
  Hatches](#escape-hatches) below for working around this.

### `*.rbi` files & missing methods

One of the most confusing errors from Sorbet can be "7003: Method does not
exist." Ruby is a very dynamic language, and methods can be defined in ways
Sorbet cannot see statically. It's possible that even though a method exists at
runtime, Sorbet cannot see it.

However, we can use `*.rbi` files to declare methods to Sorbet so that it **can**
see them statically. For more information about RBI files:

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

[receiver]: https://stackoverflow.com/questions/916572/in-ruby-what-does-the-receiver-refer-to

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

### `T.cast`

`T.unsafe` is maximally unsafe. It forces Sorbet to forget all type information
statically—sometimes this is more power than we need. For example, sometimes
we the programmer are aware of an invariant in the code that isn't currently
expressed in the type system:

```ruby
extend T::Sig

class A; def foo; end; end
class B; def bar; end; end

sig {params(label: String, a_or_b: T.any(A, B)).void}
def foo(label, a_or_b)
  case label
  when 'a'
    a_or_b.foo
  when 'b'
    a_or_b.bar
  end
end
```

In this case, we know (through careful test cases / confidence in our production
monitoring) that every time this method is called with `label = 'a'`, `a_or_b`
is an instance of `A`, and same for `'b'` / `B`.

Ideally we'd refactor our code to express this invariant in the types. To
reiterate: the **preferred** solution is to refactor this code. The time spent
adjusting this code now will make it easier and safer to refactor the code in
the future. Even still, we don't always have the time *right now*, so let's see
how we can work around the issue.

We **could** use `T.unsafe` here, but that's a pretty big hammer. Instead, we'll
use `T.cast` to explicitly tell our invariant to Sorbet:

```ruby
  case label
  when 'a'
    T.cast(a_or_b, A).foo
  when 'b'
    T.cast(a_or_b, B).bar
  end
```

Sorbet cannot **statically** guarantee that a `T.cast`-enforced invariant will
succeed in every case, but it will check the invariant **dynamically** on every
invocation.

`T.cast` is better than `T.unsafe`, because it means that something like

```ruby
    T.cast(a_or_b, A).bad_method
```

will still be caught as a missing method statically.


### `.soft()` & `.checked()`

`T.unsafe` and `T.cast` let us opt out of static checks. These next constructs
let us opt out of runtime checks. We might want to opt out of runtime checks
because...

- we've measured that calling our method is performance sensitive
- we're rolling out types to previously-untyped code, and want to make sure our
  types are correct

To opt out of runtime checks, we have three options, all of which we'll add to
the `sig` of the specific method we'd like to opt out of:

- `.soft(notify: ...)`
- `.checked(:tests)`
- `.checked(:never)`

From the static system's perspective, adding any of these to a sig will have no
effect. In the runtime, the behavior of these three differs depending on whether
we're in tests or in production:

| If types don't match, using ↓ in → ... | Tests            | Production       |
| ---                                    | -----            | ----------       |
| _none of the below_                    | will raise       | will raise       |
| `.soft(notify: ...)`                   | will raise       | soft asserts     |
| `.checked(:tests)`                     | will raise       | skips validation |
| `.checked(:never)`                     | skips validation | skips validation |

To recap:

- By default, calling a method or returning a result that doesn't match the sig
  will **raise** in both tests and prod.

- `.soft()` works the same as all soft assertions (raise in tests, soft assert
  in prod)

- `.checked(:tests)` is preferred for methods which we've measured to be
  performance sensitive. They'll have no effect on production from the runtime
  perspective, but will still be checked in tests.

- `.checked(:never)` is a hammer. It silences all validation in both tests and
  production. Use this with caution!! Without **any** runtime validation, Sorbet
  cannot warn when interoperating with untyped code.


---
id: troubleshooting
title: Troubleshooting
---

> General strategies for getting unblocked when faced with a type error.

This is one of three docs aimed at helping answer common questions about Sorbet:

- [→ Sorbet Error Reference](error-reference.md)
- [→ Ruby Types FAQ](faq.md)
- [Troubleshooting](troubleshooting.md) (this doc)

This doc covers two main topics:

1. When you're stuck, how to find out **why**
2. Regardless of why, how to **get unstuck**


## Validating your assumptions

When faced with a type error, checking your assumptions is step number one.
Here are some tools and tactics to help.

Sometimes even if you can figure out where the problem is coming from, it's not
possible for you to fix it easily. Step number two is to work around the
problem; see [Escape Hatches](#escape-hatches) below.

### `T.reveal_type`

If you wrap a variable or method call in `T.reveal_type`, Sorbet will show
you what type it thinks that variable has.

This is a super powerful debugging technique! `T.reveal_type` should be one of
the first tools you reach for when debugging a confusing error.

<!-- TODO(jez) Shorten this example and make it more of a story. -->

Try hypothesizing the problem ("I think the problem is...") and create small
examples in <https://sorbet.run> to test these problems.

For example:

```ruby
# typed: true
extend T::Helpers

sig {params(xs: T::Array[Integer]).returns(Integer)}
def foo(xs)
  T.reveal_type(xs.first) # => Revealed type: `T.nilable(Integer)`
end
```
→ View on sorbet.run

With this example we see that `xs.first` returns `T.nilable(Integer)`.

Frequently when troubleshooting type errors, either something is `nil` when or
`T.untyped` unexpectedly. We can use `T.reveal_type` to track down where the
type originated.

Remember: Sorbet is a [gradual type system]. While most code you'll encounter is
typed, that are still large swaths of code that's untyped!

### sorbet.run

Sorbet is available in an [online sandbox](https://sorbet.run). The online version of Sorbet can
typecheck...

- core Ruby language constructs (like control flow)
- Ruby standard libraries (like `Array` and `Hash`)
- **select** pay-server DSLs (like `Chalk::ODM` `prop`s)

Using [sorbet.run](https://sorbet.run) to make a minimal repro is a great way to
learn if some behavior is core to how Sorbet models Ruby, or is because of some
specific change you've introduced locally.

### Run your code

<!-- TODO(jez) Document offline sorbet runtime. -->

Types predict values. Is Sorbet's prediction accurate? Use `irb`, `pay console`,
or `pay test` to run your code. Does it actually work, even when Sorbet thinks
it doesn't? If it doesn't actually work, Sorbet caught a bug!

Otherwise, there are a handful of reasons why Sorbet predicts code will not work
even when it does:

- Maybe the code ran fine for all provided input values, but there's an edge
  case being left untested.

- Maybe the code uses a method from the standard library that is missing or
  typed incorrectly (our standard library shims are sometimes incomplete).

- Maybe a method Sorbet thinks doesn't exist actually **does** exist because it
  was dynamically defined with `define_method` or `missing_method`. See [Escape
  Hatches](#escape-hatches) below for working around this.

### Help with common errors

The tips above are very generic and apply to lots of cases. For some common
gotchas when using Sorbet, you'll want to check these two resources:

- [→ Sorbet Error Reference](error-reference.md)

  This document is a collaborative reference with suggestions for commonly
  encountered error codes. You should see links to this document in Sorbet's
  error output.

- [→ Ruby Types FAQ](faq.md)

  This is a list of questions people commonly have when working with Sorbet and
  the runtime type system. Skim it to see if it says anything about what you're
  working on!


## Escape Hatches

<!-- TODO(jez) Note about how you should always prefer the refactor. -->

Regardless of whether you can figure out the root cause of the error at hand,
Sorbet is designed as a [gradual type system]. This means there will always be
**escape hatches** to silence the problem.

### `T.unsafe`

By wrapping an expression like `x` in `T.unsafe(...)` you can ask Sorbet to
forget the result type of something.

One case when this is useful when you know for sure that a method exists, but
Sorbet don't know about that method exists statically:

```
missing method example

T.unsafe(self) too
```


### `T.cast`

`T.unsafe` is maximally unsafe. It forces Sorbet to forget all type information
statically—sometimes this is more power than you need. For example, sometimes
you the programmer are aware of an invariant in the code that isn't currently
expressed in the type system:

```ruby
extend T::Helpers

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
re-iterate: the **preferred** solution is to refactor this code. The time spent
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

From the static system's perspective, these adding any of these to a sig will
have no effect. In the runtime, the behavior of these three differs depending on
whether we're in tests or in production:

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

- `.checked(:tests)` is preferred for methods which you've measured to be
  performance sensitive. They'll have no effect on production from the runtime
  perspective, but will still be checked in tests.

- `.checked(:never)` is a hammer. It silences all validation in both tests and
  production. Use this with caution!! Without **any** runtime validation, Sorbet
  cannot warn when interoperating with untyped code.


[gradual type system]: /blog/2018/01/09/gradual-type-systems

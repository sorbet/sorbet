---
id: nilable-types
title: Nilable Types
sidebar_label: Nilable Types (T.nilable)
---

Sorbet can track when a value is allowed to be `nil`, or when a value of a
certain type must be present. In Sorbet, such a type is called a **nilable**
type. In Sorbet, types are non-nil by default. We have to explicitly opt a type
into allowing `nil` by wrapping it in `T.nilable(...)`:

```ruby
T.nilable(String)
```

Valid values for this type are either `nil` or any Ruby string. Once we have
something of a nilable type, we can use it like this:

```ruby
extend T::Sig

sig {params(x: T.nilable(String)).void}
def foo(x)
  if x
    puts "x was not nil! Got: #{x}"
  else
    puts "x was nil"
  end
end
```

## Tracking `nil`

Sorbet is smart enough to follow the control flow of a program to update its
knowledge about what types each variable has within each branch. For example, if
we have a method that declares it can only be given something that's not `nil`,
Sorbet will force us to first check whether our variable is `nil`:

```ruby
extend T::Sig

sig {params(x: String).void}
def must_be_given_string(x)
  puts "Got string: #{x}"
end

sig {params(x: T.nilable(String)).void}
def foo(x)
  must_be_given_string(x) # error: Expected `String` but found `T.nilable(String)` for argument `x`
  if x
    must_be_given_string(x) # ok
  end
end
```

<a href="https://sorbet.run/#extend%20T%3A%3ASig%0A%0Asig%20%7Bparams(x%3A%20String).void%7D%0Adef%20must_be_given_string(x)%0A%20%20puts%20%22Got%20string%3A%20%23%7Bx%7D%22%0Aend%0A%0Asig%20%7Bparams(x%3A%20T.nilable(String)).void%7D%0Adef%20foo(x)%0A%20%20must_be_given_string(x)%20%23%20error%3A%20%0A%20%20if%20x%0A%20%20%20%20must_be_given_string(x)%20%23%20ok%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

This is one of the most useful features of Sorbet. Many bugs can be prevented by
ensuring when something is `nil` and when something is not `nil`! For more
information on how this control flow-sensitivity works and how to take full
advantage of it, see [Flow-sensitive Typing](flow-sensitive.md).

## `T.must`: Asserting that something is not `nil`

When Sorbet reports an error about a type mismatch, we strongly encourage
thinking through what it means. For example, in which situations could this be
`nil`? Is there some sensible behavior when given `nil`? Sometimes accepting
`nil` makes sense, and other times we'd like to never take something that's nil
and request that the caller filter out `nil` values before even calling our
method.

However, sometimes either

- we're sure this thing can never be nil, or
- it isn't valuable to spend the time handling the `nil` case.

In cases like these, we can use `T.must` to silence the error. Let's walk
through an example to see how it works. Consider this code with an error:

```ruby
# typed: true
extend T::Sig

sig {params(x: Integer).void}
def doesnt_take_nil(x); end

sig {params(key: Symbol, options: T::Hash[Symbol, Integer]).void}
def foo(key, options)
  val = options[key]
  doesnt_take_nil(val) # error: Expected `Integer` but found `T.nilable(Integer)` for argument `x`
end
```

In this example, `foo` accepts a Hash with `Symbol` keys and `Integer` values.
We looked up the element in the Hash at key `key`, got back `val`, and passed it
to `doesnt_take_nil`. Here, Sorbet complains, because `val` could be `nil` (if
the key doesn't exist in the hash).

In general, there's no way to know whether `key` is in `options`, but we might
have special knowledge (that Sorbet doesn't know) to convince us that `val` will
never be `nil`. Maybe:

- this code runs in production very frequently, with no issues.
- our test suite has good coverage for this code.
- we validate that `key` is a valid key somewhere higher up in our code.

When we're **sure** that `val` must never be `nil`, we can wrap it in
`T.must(...)`:

```ruby
# typed: true
extend T::Sig

sig {params(x: Integer).void}
def doesnt_take_nil(x); end

sig {params(key: Symbol, options: T::Hash[Symbol, Integer]).void}
def foo(key, options)
  val = T.must(options[key])
  doesnt_take_nil(val) # ok
end
```

Using `T.must` is akin to saying "Sorbet, please trust me: at runtime this value
will never be `nil`." In essence, we're trading off static guarantees for
runtime guarantees. Put another way, we're shifting the "burden of proof" for
this code's correctness from Sorbet to the programmer, and we as programmers can
"prove" that this code is not nil using whatever means is convenient (tests,
observability, etc.). In fact, it's the same tradeoff we make every time we
`raise` an exception, and comes with the same set of caution labels:

→ For more information, see the [`T.must` docs](type-assertions.md#tmust).

> **Note**: Like all other type checks in Sorbet, `T.must` will raise at runtime
> if it fails. For more information, see [Enabling Runtime Checks](runtime.md).

## Alternatives to `T.must`

Sometimes `T.must` can "clutter up" code, so here are some alternatives that
accomplish the same thing as or something similar to `T.must`.

### `Array#fetch` & `Hash#fetch`

The Ruby standard library has a couple built-in methods for raising an exception
if an element is missing from a collection: `x.fetch(...)`:

```ruby
# Our `T.must` from the previous section's example:
val = T.must(options[key])

# The same thing, but with `fetch`:
val = options.fetch(key)
```

Like `T.must`, `fetch` will raise if the key is not found. Using `fetch` is
convenient because we can also provide a default value to use when the key
doesn't exist:

```ruby
# Use `0` for `val` if key is not found:
val = options.fetch(key, 0)
```

Written this way, this line will never raise an exception and `val` will never
be `nil`.

### `&.`: The safe navigation operator

Ruby 2.3 added special syntax to the language to do something like `x&.foo`,
which means "call `foo` if `x` is not `nil`, otherwise short circuit and
evaluate to `nil`." This is similar to `T.must`, but not quite the same.
Consider:

```ruby
# (1)
val = T.must(x).foo

# (2)
val = x&.foo
```

In (1), if `x` is nil the code will raise an exception, and `val` will never be
`nil`. But in (2), the code will not raise an exception, but `val` might be
`nil`. Here's a longer example:

```ruby
extend T::Sig

sig {params(x: T.nilable(Integer)).returns(Integer)}
def foo(x)
  y = T.must(x).abs
  T.reveal_type(y)
end

sig {params(x: T.nilable(Integer)).returns(T.nilable(Integer))}
def bar(x)
  y = x&.abs
  T.reveal_type(y)
end
```

<a href="https://sorbet.run/#extend%20T%3A%3ASig%0A%0Asig%20%7Bparams(x%3A%20T.nilable(Integer)).returns(Integer)%7D%0Adef%20foo(x)%0A%20%20y%20%3D%20T.must(x).abs%0A%20%20T.reveal_type(y)%0Aend%0A%0Asig%20%7Bparams(x%3A%20T.nilable(Integer)).returns(T.nilable(Integer))%7D%0Adef%20bar(x)%0A%20%20y%20%3D%20x%26.abs%0A%20%20T.reveal_type(y)%0Aend">
  → View on sorbet.run
</a>

### Other escape hatches

`T.must` is one of the handful of escape hatches in Sorbet. For more
information, see [escape hatches](troubleshooting.md#escape-hatches).

Also, people frequently confuse `T.must` with `T.let`, `T.cast`, and `T.unsafe`.
Each of these four are actually rather different; for the differences, see
[Type Assertions](type-assertions.md).

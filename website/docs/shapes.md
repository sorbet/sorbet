---
id: shapes
title: Shapes
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
{symbol1: Type1, symbol2: Type2}

{'string1' => Type1, 'string2' => Type2}
```

This creates a fixed hash type (also referred to as a record), which is a hash with known keys and known types for each key. For example, `{foo: String, bar: T.nilable(Float)}` validates that an object is a hash with exactly those 2 keys as Ruby symbols, with `foo` being a `String` and `bar` being a `Float` or `nil`. For example: `{foo: 'hello', bar: 3.14}`.

> **Warning**: Shape types have many known limitations, and should be considered an experimental feature. They may not work as expected or change without notice. For an alternative that plays better with static type checking, see [Typed Structs](tstruct.md).

```ruby
# typed: true

# Shape types work for some simple cases,
# but have many known limitations.
#
# Consider instead using T::Struct to create a
# data class which models your domain.
#

extend T::Sig

sig { params(x: {a: Integer, b: String}).void }
def foo(x)
  # Limitation! returns T.untyped!
  T.reveal_type(x[:a]) # Revealed type: `T.untyped`

  # Limitation! does not warn when key doesn't exist!
  x[:c]
end

# --- What you expect ---
foo({a: 0,  b: ''}) # ok
foo({a: '', b: 0}) # error: type mismatch
foo({}) # error: missing keys

# --- What you might not expect ---
foo({a: 0, b: '', j: nil}) # ok

# --- Mutation: the ugly ---
y = {a: 0, b: ''}
y[:a] = '' # ok (!)
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0A%23%20Shape%20types%20work%20for%20some%20simple%20cases%2C%0A%23%20but%20have%20many%20known%20limitations.%0A%23%0A%23%20Consider%20instead%20using%20T%3A%3AProps%20%2F%20T%3A%3AStruct%20to%20create%20a%0A%23%20data%20class%20which%20models%20your%20domain.%0A%23%0A%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams%28x%3A%20%7Ba%3A%20Integer%2C%20b%3A%20String%7D%29.void%7D%0Adef%20foo%28x%29%0A%20%20%23%20Limitation!%20returns%20T.untyped!%0A%20%20T.reveal_type%28x%5B%3Aa%5D%29%20%23%20Revealed%20type%3A%20%60T.untyped%60%0A%0A%20%20%23%20Limitation!%20does%20not%20warn%20when%20key%20doesn't%20exist!%0A%20%20x%5B%3Ac%5D%0Aend%0A%0A%23%20---%20What%20you%20expect%20---%0Afoo%28%7Ba%3A%200%2C%20%20b%3A%20''%7D%29%20%23%20ok%0Afoo%28%7Ba%3A%20''%2C%20b%3A%200%7D%29%20%23%20error%3A%20type%20mismatch%0Afoo%28%7B%7D%29%20%23%20error%3A%20missing%20keys%0A%0A%23%20---%20What%20you%20might%20not%20expect%20---%0Afoo%28%7Ba%3A%200%2C%20b%3A%20''%2C%20j%3A%20nil%7D%29%20%23%20ok%0A%0A%23%20---%20Mutation%3A%20the%20ugly%20---%0Ay%20%3D%20%7Ba%3A%200%2C%20b%3A%20''%7D%0Ay%5B%3Aa%5D%20%3D%20''%20%23%20ok%20%28!%29%0A">
  → View on sorbet.run
</a>

## Hash literals

Poor support for shapes even pollutes Sorbet's support for [typed hashes](stdlib-generics.md):

```ruby
opts = {symbol_key: 0}

# This should not type check (Symbol is not String),
# but Sorbet reports no error:
T.let(
  opts,
  T::Hash[String, Integer]
)

T.reveal_type(opts) # => `{symbol_key: Integer(0)} (shape of T::Hash[T.untyped, T.untyped])`
```

Hash literals decay to `T::Hash[T.untyped, T.untyped]`, instead of decaying to a union of their keys' and values' types. This is tied to hard-to-change implementation choices in Sorbet's inference algorithm. We will change it one day.

In the mean time, to explicitly create a typed hash, use one of these options:

```ruby
# Empty, typed hash:
T::Hash[String, Integer].new

# Hash literal to typed hash:
{symbol_key: 0}.to_h
```

In both cases, the expression will be eagerly upcast to a typed hash: Sorbet forgets that the expression is a hash with specific keys:

```ruby
sig { params(x: {symbol_key: Integer}).void }
def takes_shape(x)
  x[:symbol_key]
end

takes_shape({symbol_key: 0}.to_h) # error: expected shape, got typed hash
```

For more, see <https://github.com/sorbet/sorbet/issues/11>.

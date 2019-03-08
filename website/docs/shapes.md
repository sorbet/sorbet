---
id: shapes
title: Shapes
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
{symbol1: Type1, symbol2: Type2}

{'string1' => Type1, 'string2' => Type2}
```

This creates a fixed hash type (also referred to as a record), which is a hash
with known keys and known types for each key. For example, `{foo: String, bar:
T.nilable(Float)}` validates that an object is an hash with exactly those 2 keys
as Ruby symbols, with `foo` being a `String` and `bar` being a `Float` or `nil`.
For example: `{foo: 'hello', bar: 3.14}`.

> **Warning**: Shape types have many known limitations, and should be considered
> an experimental feature. They may not work as expected or change without
> notice.

```ruby
##
# Shape types work for some simple cases,
# but have many known limitations.
#
# Consider instead using T::Struct to create a
# data class which models your domain.
#

extend T::Sig

sig {params(x: {a: Integer, b: String}).void}
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

[→ View on sorbet.run](https://sorbet.run/#%23%0A%23%20Shape%20types%20work%20for%20some%20simple%20cases%2C%0A%23%20but%20have%20many%20known%20limitations.%0A%23%0A%23%20Consider%20instead%20using%20T%3A%3AProps%20%2F%20T%3A%3AStruct%20to%20create%20a%0A%23%20data%20class%20which%20models%20your%20domain.%0A%23%0A%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(x%3A%20%7Ba%3A%20Integer%2C%20b%3A%20String%7D).void%7D%0Adef%20foo(x)%0A%20%20%23%20Limitation!%20returns%20T.untyped!%0A%20%20T.reveal_type(x%5B%3Aa%5D)%20%23%20Revealed%20type%3A%20%60T.untyped%60%0A%0A%20%20%23%20Limitation!%20does%20not%20warn%20when%20key%20doesn't%20exist!%0A%20%20x%5B%3Ac%5D%0Aend%0A%0A%23%20---%20What%20you%20expect%20---%0Afoo(%7Ba%3A%200%2C%20%20b%3A%20''%7D)%20%23%20ok%0Afoo(%7Ba%3A%20''%2C%20b%3A%200%7D)%20%23%20error%3A%20type%20mismatch%0Afoo(%7B%7D)%20%23%20error%3A%20missing%20keys%0A%0A%23%20---%20What%20you%20might%20not%20expect%20---%0Afoo(%7Ba%3A%200%2C%20b%3A%20''%2C%20j%3A%20nil%7D)%20%23%20ok%0A%0A%23%20---%20Mutation%3A%20the%20ugly%20---%0Ay%20%3D%20%7Ba%3A%200%2C%20b%3A%20''%7D%0Ay%5B%3Aa%5D%20%3D%20''%20%23%20ok%20(!)%0A)

sorbet internals note: the underlying of a shape is an Hash where the key type
is the union of all keys and the value type is the union of all the values.

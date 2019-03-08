---
id: tuples
title: Tuples
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
[Type1, Type2, ...]
```

This creates a fixed array type (also referred to as a tuple), which is a
fixed-length array with known types for each element. For example, `[String,
T.nilable(Float)]` validates that an object is an array of exactly length 2,
with the first item being a `String` and the second item being a `Float` or
`nil`.

> **Warning**: Tuples have many known limitations, and should be considered an
> experimental feature. They may not work as expected or change without notice.

```ruby
##
# Tuple types work for some simple cases,
# but have many known limitations.
#

extend T::Sig

sig {params(x: [Integer, String]).returns(Integer)}
def foo(x)
  T.reveal_type(x[0]) # Revealed type: `Integer`
end

# --- What you expect ---
foo([0, '']) # ok
foo(['', 0]) # error: type mismatch
foo([]) # error: not right tuple type

# --- What you might not expect ---
foo([0, '', nil]) # ok

# --- Mutation: the ugly ---
y = [0, '']
y[0] = '' # ok (!)
T.reveal_type(y[0]) # Reveal type: `Integer(0)` (!!)

# --- Flow-sensitivity: even uglier ---
y_0 = y[0]
if y_0.is_a?(String)
  puts y_0 # error: This code is unreachable (!!!)
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%0A%23%20Tuple%20types%20work%20for%20some%20simple%20cases%2C%0A%23%20but%20have%20many%20known%20limitations.%0A%23%0A%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams(x%3A%20%5BInteger%2C%20String%5D).returns(Integer)%7D%0Adef%20foo(x)%0A%20%20T.reveal_type(x%5B0%5D)%20%23%20Revealed%20type%3A%20%60Integer%60%0Aend%0A%0A%23%20---%20What%20you%20expect%20---%0Afoo(%5B0%2C%20''%5D)%20%23%20ok%0Afoo(%5B''%2C%200%5D)%20%23%20error%3A%20type%20mismatch%0Afoo(%5B%5D)%20%23%20error%3A%20not%20right%20tuple%20type%0A%0A%23%20---%20What%20you%20might%20not%20expect%20---%0Afoo(%5B0%2C%20''%2C%20nil%5D)%20%23%20ok%0A%0A%23%20---%20Mutation%3A%20the%20ugly%20---%0Ay%20%3D%20%5B0%2C%20''%5D%0Ay%5B0%5D%20%3D%20''%20%23%20ok%20(!)%0AT.reveal_type(y%5B0%5D)%20%23%20Reveal%20type%3A%20%60Integer(0)%60%20(!!)%0A%0A%23%20---%20Flow-sensitivity%3A%20even%20uglier%20---%0Ay_0%20%3D%20y%5B0%5D%0Aif%20y_0.is_a%3F(String)%0A%20%20puts%20y_0%20%23%20error%3A%20This%20code%20is%20unreachable%20(!!!)%0Aend%0A)


sorbet internals note: the underlying of a tuple is an Array of the union of
each element type.


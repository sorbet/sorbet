---
id: type-assertions
title: Type Assertions
---

There are five ways to assert the types of expressions in Sorbet:

- `T.let(expr, Type)`
- `T.cast(expr, Type)`
- `T.must(expr)`
- `T.assert_type!(expr, Type)`
- `T.unsafe(expr)`

> `T.unsafe` is a cast to `T.untyped`, so it's more of an
> [Escape Hatch](troubleshooting.md#escape-hatches).

## `T.let`

A `T.let` assertion is checked statically **and** at runtime. In the following
example, the definition of `y` will raise an error when Sorbet is run, and also
when the program is run.

```ruby
x = T.let(10, Integer)
T.reveal_type(x) # Revealed type: Integer

y = T.let(10, String) # error: Argument does not have asserted type String
```
<a href="https://sorbet.run/#%23%20typed%3A%20true%0Ax%20%3D%20T.let(10%2C%20Integer)%0AT.reveal_type(x)%20%23%20Revealed%20type%3A%20Integer%0A%0Ay%20%3D%20T.let(10%2C%20String)%20%23%20error%3A%20Argument%20does%20not%20have%20asserted%20type%20String">
  → View on sorbet.run
</a>

## `T.cast`

A `T.cast` assertion is only checked at runtime. Statically, Sorbet assumes this
assertion is always true. This can be used to change the result type of an
expression from the perspective of the static system. In the example below,
Sorbet will not report any problems with the definition of `y`, while an error
will be raised at runtime:

```ruby
x = T.cast(10, Integer)
T.reveal_type(x) # Revealed type: Integer

y = T.cast(10, String) # OK statically, but will raise an error at runtime!
T.reveal_type(y) # Revealed type: String
```
<a href="https://sorbet.run/#%23%20typed%3A%20true%0Ax%20%3D%20T.cast(10%2C%20Integer)%0AT.reveal_type(x)%20%23%20Revealed%20type%3A%20Integer%0A%0Ay%20%3D%20T.cast(10%2C%20String)%20%23%20OK%20statically%2C%20but%20will%20raise%20an%20error%20at%20runtime!%0AT.reveal_type(y)%20%23%20Revealed%20type%3A%20String">
  → View on sorbet.run
</a>

## `T.must`

`T.must` is for asserting that a value of a [nilable type](nilable-types.md) is
not `nil`. `T.must` is similar to `T.cast` in that it will not necessarily
trigger an error when Sorbet is run, but can trigger an error during runtime.
The following example illustrates two cases:

1. a use of `T.must` with a value that Sorbet is able to determine statically
   is `nil`, that raises an error indicating that the subsequent statements are
   unreachable;
2. a use of `T.must` with a computed `nil` value that Sorbet is not able to
   detect statically, which raises an error at runtime.

```ruby
class A
  extend T::Sig

  sig {void}
  def foo
    x = T.let(nil, T.nilable(String))
    y = T.must(nil)
    puts y # error: This code is unreachable
  end

  sig {void}
  def bar
    vals = T.let([], T::Array[Integer])
    x = vals.find {|a| a > 0}
    T.reveal_type(x) # Revealed type: T.nilable(Integer)
    y = T.must(x)
    puts y # no static error
  end

end
```
<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20foo%0A%20%20%20%20x%20%3D%20T.let(nil%2C%20T.nilable(String))%0A%20%20%20%20y%20%3D%20T.must(nil)%0A%20%20%20%20puts%20y%20%23%20error%3A%20This%20code%20is%20unreachable%0A%20%20end%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20bar%0A%20%20%20%20vals%20%3D%20T.let(%5B%5D%2C%20T%3A%3AArray%5BInteger%5D)%0A%20%20%20%20x%20%3D%20vals.find%20%7B%7Ca%7C%20a%20%3E%200%7D%0A%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20T.nilable(Integer)%0A%20%20%20%20y%20%3D%20T.must(x)%0A%20%20%20%20puts%20y%20%23%20no%20static%20error%0A%20%20end%0A%0Aend">
  → View on sorbet.run
</a>

## `T.assert_type!`

`T.assert_type!` is similar to `T.let`: it is checked statically **and** at
runtime. It has the additional restriction that it will **always** fail
statically if given something that's [`T.untyped`](untyped.md). For example:

```ruby
class A
  extend T::Sig

  sig {params(x: T.untyped).void}
  def foo(x)
    T.assert_type!(x, String) # error here
  end
end
```
<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.untyped).void%7D%0A%20%20def%20foo(x)%0A%20%20%20%20T.assert_type!(x%2C%20String)%20%23%20error%20here%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

## Comparison of type assertions

Some other ways to think about it:

- `T.let` vs `T.cast`

  ```ruby
  T.cast(expr, Type)
  ```

  is the same as

  ```ruby
  T.let(T.unsafe(expr), Type)
  ```

- `T.unsafe` in terms of `T.let`

  ```ruby
  T.unsafe(expr)
  ```

  is the same as

  ```ruby
  T.let(expr, T.untyped)
  ```

- `T.must` is like `T.cast`, but without having to know the result type:

  ```ruby
  T.cast(nil_or_string, String)
  ```

  is the same as

  ```ruby
  T.must(nil_or_string)
  ```

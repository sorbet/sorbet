---
id: faq
title: Frequently Asked Questions
sidebar_label: FAQ
---

## Why does Sorbet think this is `nil`? I just checked that it's not!

Sorbet implements a [flow-sensitive](flow-sensitive.md) type system, but there
are some [limitations](flow-sensitive.md#limitations-of-flow-sensitivity). In
particular, Sorbet does not assume a method called twice returns the same thing
each time!

See [Limitations of
flow-sensitivity](flow-sensitive.md#limitations-of-flow-sensitivity) for a fully
working example.

## What's the difference between `T.let`, `T.cast`, and `T.unsafe`?

[→ Type Assertions](type-assertions.md)

## What's the type signature for a method with no return?

```ruby
sig {void}
```

[→ Method Signatures](sigs.md)


## What's the difference between `Array` and `T::Array[…]`?

`Array` is the built-in Ruby class for arrays. On the other hand,
`T::Array[...]` is a Sorbet generic type for arrays. The `...` must always be
filled in when using it.

While Sorbet implicitly treats `Array` the same as `T::Array[T.untyped]`,
Sorbet will error when trying to use `T::Array` as a standalone type.

[→ Generics in the Standard Library](stdlib-generics.md)


## How do I accept a class object, and not an instance of a class?

[→ T.class_of](class-of.md)


## How do I write a signature for `initialize`?

When defining `initialize` for a class, we strongly encourage that you use
`.void`. This reminds people instantiating your class that they probably meant
to call `.new`, which is defined on every Ruby class. Typing the result as
`.void` means that it's not possible to do anything with the result of
`initialize`.

[→ Method Signatures](sigs.md)

## How do I override `==`? What signature should I use?

Your method should accept `BasicObject` and return `T::Boolean`.

Unfortunately, not all `BasicObject`s have `is_a?`, so we have to do one extra
step in our `==` function: check whether `Object === other`. (In Ruby, `==` and
`===` are completely unrelated. The latter has to do with [case subsumption]).
The idiomatic way to write `Object === other` in Ruby is to use `case`:

```ruby
case other
when Object
  # ...
end
```

[case subsumption]: https://stackoverflow.com/questions/3422223/vs-in-ruby

Here's a complete example that uses `case` to implement `==`:

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20IntBox%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns(Integer)%7D%0A%20%20attr_reader%20%3Aval%0A%20%20%0A%20%20sig%20%7Bparams(val%3A%20Integer).void%7D%0A%20%20def%20initialize(val)%0A%20%20%20%20%40val%20%3D%20val%0A%20%20end%0A%20%20%0A%20%20sig%20%7Bparams(other%3A%20BasicObject).returns(T::Boolean)%7D%0A%20%20def%20%3D%3D(other)%0A%20%20%20%20case%20other%0A%20%20%20%20when%20IntBox%0A%20%20%20%20%20%20other.val%20%3D%3D%20val%0A%20%20%20%20else%0A%20%20%20%20%20%20false%0A%20%20%20%20end%0A%20%20end%0Aend">→ View on sorbet.run</a>


## I use `T.must` a lot with arrays and hashes. Is there a way around this?

`Hash#[]` and `Array#[]` return a [nilable type](nilable-types.md) because in
Ruby, accessing a Hash or Array returns `nil` if the key does not exist or is
out-of-bounds. If you would rather raise an exception than handle `nil`, use
the `#fetch` method:

```ruby
[0, 1, 2].fetch(3) # IndexError: index 3 outside of array bounds
```


---
id: union-types
title: Union Types
sidebar_label: Union Types (T.any)
---

Union types declare that a value either has one type, or some other type. The
basic syntax for `T.any` is:

```ruby
T.any(SomeType, SomeOtherType, ...)
```

For example, `T.any(Integer, String)` describes a type whose values can be
either `Integer` or `String` values, but no others.

```ruby
class A
  extend T::Sig

  sig {params(x: T.any(Integer,String)).void}
  def self.foo(x); end
end

# 10 and "Hello, world" both have type `T.any(Integer, String)`
A.foo(10)
A.foo("Hello, world")

# error: Expected `T.any(Integer, String)` but found `TrueClass`
A.foo(true)
```

<a href="https://sorbet.run/#%23%20typed%3A%20True%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.any(Integer%2CString)).void%7D%0A%20%20def%20self.foo(x)%3B%20end%0Aend%0A%0A%23%2010%20and%20%22Hello%2C%20world%22%20both%20have%20type%20%60T.any(Integer%2C%20String)%60%0AA.foo(10)%0AA.foo(%22Hello%2C%20world%22)%0A%0A%23%20%60TrueClass%60%20does%20not%20match%20%60T.any(Integer%2C%20String)%60%0AA.foo(true)">
  → View on sorbet.run
</a>

## Union types and flow-sensitivity

Given a value `x` with a type like `T.any(Integer, String)`, Sorbet will only
allow calls to methods that both types have in common, like this:

```ruby
sig {params(x: T.any(Integer, String)).void}
def example(x)
  # both `Integer` and `String` have a `to_s` method, so this is okay
  puts(x.to_s)
end
```

But sometimes we want to be able to call a method that only exists on one of
those two types. For example, `Integer` has an `even?` method that doesn't exist
on `String`. If we didn't do anything special, Sorbet would report an error:

```ruby
sig {params(x: T.any(Integer, String)).void}
def example(x)
  # ERROR: Method `even?` does not exist on `String` component of `T.any(Integer, String)`
  x.even?
end
```

In situations like these, we have to first check whether `x` is an `Integer` or
not, and only then call the method:

```ruby
sig {params(x: T.any(Integer, String)).void}
def example(x)
  if x.is_a?(Integer)
    x.even? # OK, because we checked with `is_a?`
  end
end
```

Sorbet is smart enough to understand many different kinds of Ruby control flow
constructs (more than just `if` statements and calls to `is_a?`). Read the
[flow-sensitive typing](flow-sensitive.md) section for a deeper dive on this
topic.

## Enumerations

Union types can be used to express enumerations. For example, if we have three
classes `A`, `B`, and `C`, and would like to make one type that describes these
three cases, `T.any(A, B, C)` is a good option:

```ruby
class A; end
class B; end
class C;
  extend T::Sig

  sig {void}
  def bar; end
end

class D
  extend T::Sig

  sig {params(x: T.any(A, B, C)).void}
  def foo(x)
    x.bar # error: method bar does not exist on A or B

    case x
    when A, B
      T.reveal_type(x) # Revealed type: T.any(B, A)
    else
      T.reveal_type(x) # Revealed type: C
      x.bar # OK, x is known to be an instance of C
    end
  end
end
```

<a
href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%3B%20end%0Aclass%20B%3B%20end%0Aclass%20C%3B%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bvoid%7D%0A%20%20def%20bar%3B%20end%0Aend%0A%0Aclass%20D%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.any(A%2C%20B%2C%20C)).void%7D%0A%20%20def%20foo(x)%0A%20%20%20%20x.bar%20%23%20error%3A%20method%20bar%20does%20not%20exist%20on%20A%20or%20B%0A%0A%20%20%20%20case%20x%0A%20%20%20%20when%20A%2C%20B%0A%20%20%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20T.any(B%2C%20A)%0A%20%20%20%20else%0A%20%20%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20C%0A%20%20%20%20%20%20x.bar%20%23%20OK%2C%20x%20is%20known%20to%20be%20an%20instance%20of%20C%0A%20%20%20%20end%0A%20%20end%0Aend">
→ View on sorbet.run </a>

In cases like this where the classes in the union don't actually carry around
any extra data, Sorbet has an even more convenient way to define enumerations.
See [Typed Enumerations via T::Enum](tenum.md).

Note that enumerations using primitive or literal types is not supported. For
example, the following is _not_ valid:

```ruby
class A
  extend T::Sig

  # ERROR, intentionally unsupported
  sig { params(input_param: T.any('foo', 'bar')).void }
  def a(input_param)
    puts input_param
  end
end
```

<a
href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7B%20params%28input_param%3A%20T.any%28'foo'%2C%20'bar'%29%29.void%20%7D%0A%20%20def%20a%28input_param%29%0A%20%20%20%20puts%20input_param%0A%20%20end%0Aend">
→ View on sorbet.run </a>

## `T.nilable` and `T::Boolean`

`T.nilable` and `T::Boolean` are both defined in terms of `T.any`:

- `T.nilable(x)` is a type constructor that will return `T.any(NilClass, x)`
- `T::Boolean` is a type alias to `T.any(TrueClass, FalseClass)`

An effect of this implementation choice is that the same information propagation
behavior outlined in
[Union types and flow sensitivity](#union-types-and-flow-sensitivity) will take
place for nilable types and booleans, as with any other union type:

```ruby
class A
  extend T::Sig

  sig {params(x: T.nilable(T::Boolean)).void}
  def foo(x)
    if x.nil?
      T.reveal_type(x) # Revealed type: NilClass
    else
      T.reveal_type(x) # Revealed type: T::Boolean
      if x
        T.reveal_type(x) # Revealed type: TrueClass
      else
        T.reveal_type(x) # Revealed type: FalseClass
      end
    end
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.nilable(T%3A%3ABoolean)).void%7D%0A%20%20def%20foo(x)%0A%20%20%20%20if%20x.nil%3F%0A%20%20%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20NilClass%0A%20%20%20%20else%0A%20%20%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20T%3A%3ABoolean%0A%20%20%20%20%20%20if%20x%0A%20%20%20%20%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20TrueClass%0A%20%20%20%20%20%20else%0A%20%20%20%20%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20FalseClass%0A%20%20%20%20%20%20end%0A%20%20%20%20end%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

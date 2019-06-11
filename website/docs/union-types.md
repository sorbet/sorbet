---
id: union-types
title: Union Types
---

Union types, introduced by `T.any`, are how we merge the values of a set of
types into one new type. The basic syntax for `T.any` is:

```
T.any(SomeType, SomeOtherType, ...)
```

Note that `T.any` requires at least two type arguments.

For example, `T.any(Integer, String)` describes a type whose values can be either
`Integer` or `String` values, but no others.

```ruby
class A
  extend T::Sig

  sig {params(x: T.any(Integer,String)).void}
  def self.f(x); end
end

# 10 and "Hello, world" both have type `T.any(Integer, String)`
A.f(10)
A.f("Hello, world")

# `TrueClass` does not match `T.any(Integer, String)`
A.f(true)
```

<a href="https://sorbet.run/#%23%20typed%3A%20True%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.any(Integer%2CString)).void%7D%0A%20%20def%20self.f(x)%3B%20end%0Aend%0A%0A%23%2010%20and%20%22Hello%2C%20world%22%20both%20have%20type%20%60T.any(Integer%2C%20String)%60%0AA.f(10)%0AA.f(%22Hello%2C%20world%22)%0A%0A%23%20%60TrueClass%60%20does%20not%20match%20%60T.any(Integer%2C%20String)%60%0AA.f(true)">
  → View on sorbet.run
</a>

## Refining union types

Union types can be refined through the use of `Object#is_a?` in a conditional
statement, or the ruby `case` statement. When using `Object#is_a?` in a
conditional, information learned by that test will be propagated down to
branches of the conditional. For example:

```ruby
class A
  extend T::Sig

  sig {params(x: T.any(String, Integer, TrueClass)).void}
  def f(x)
    # Revealed type: T.any(String, Integer, T::Boolean)
    T.reveal_type(x)
    if x.is_a?(String) or x.is_a?(Integer)
      # Revealed type: T.any(String, Integer)
      T.reveal_type(x)
    else
      # Revealed type: TrueClass
      T.reveal_type(x)
    end
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.any(String%2C%20Integer%2C%20TrueClass)).void%7D%0A%20%20def%20f(x)%0A%20%20%20%20%23%20Revealed%20type%3A%20T.any(String%2C%20Integer%2C%20T%3A%3ABoolean)%0A%20%20%20%20T.reveal_type(x)%0A%20%20%20%20if%20x.is_a%3F(String)%20or%20x.is_a%3F(Integer)%0A%20%20%20%20%20%20%23%20Revealed%20type%3A%20T.any(String%2C%20Integer)%0A%20%20%20%20%20%20T.reveal_type(x)%0A%20%20%20%20else%0A%20%20%20%20%20%20%23%20Revealed%20type%3A%20TrueClass%0A%20%20%20%20%20%20T.reveal_type(x)%0A%20%20%20%20end%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

Similarly, any types specified in the `when` clause of a `case` statement will
refine the type of the expression being analyzed within the context of that
`when` branch:

```ruby
class A
  extend T::Sig

  sig {params(x: T.any(String, Integer, TrueClass)).void}
  def f(x)
    # Revealed type: T.any(String, Integer, TrueClass)
    T.reveal_type(x)
    case x
    when Integer, String
      # Revealed type: T.any(Integer, String)
      T.reveal_type(x)
    else
      # Revealed type: TrueClass
      T.reveal_type(x)
    end
  end
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.any(String%2C%20Integer%2C%20TrueClass)).void%7D%0A%20%20def%20f(x)%0A%20%20%20%20%23%20Revealed%20type%3A%20T.any(String%2C%20Integer%2C%20TrueClass)%0A%20%20%20%20T.reveal_type(x)%0A%20%20%20%20case%20x%0A%20%20%20%20when%20Integer%2C%20String%0A%20%20%20%20%20%20%23%20Revealed%20type%3A%20T.any(Integer%2C%20String)%0A%20%20%20%20%20%20T.reveal_type(x)%0A%20%20%20%20else%0A%20%20%20%20%20%20%23%20Revealed%20type%3A%20TrueClass%0A%20%20%20%20%20%20T.reveal_type(x)%0A%20%20%20%20end%0A%20%20end%0Aend">
  → View on sorbet.run
</a>

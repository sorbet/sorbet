---
id: nilable-types
title: Nilable Types
---

> TODO(jez) This page is still a fragment. Contributions welcome!

Example:

```
T.nilable(String)
```

Ties into [Flow-Sensitive Typing](flow-sensitive.md) to check whether something
is `nil` or not.

- Standard library: `Hash#[]` and `Array#[]` return `T.nilable` (returns `nil`
  if element is not there)
  - if you'd rather it raise an exception:
    - `Array#fetch` / `Hash#fetch`
    - `T.must` (see also: [Inline Type Assertions](inline.md))

Editorial note: something along the lines of "trading off static guarantees for
runtime guarantees." This is a tradeoff, and sometimes it makes sense, sometimes
it doesn't. (Burden of proof shifts, from Sorbet to programmer; need tests /
observability etc.) This is the same tradeoff as using `raise` in the first
place.

Difference between `T.must` and `&.` ?

- `T.must(x).foo` either returns `x.foo` or raises
- `x&.foo` either returns `x.foo` or returns `nil`

Example:

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

[→ View on sorbet.run](https://sorbet.run/#extend%20T%3A%3ASig%0A%0Asig%20%7Bparams(x%3A%20T.nilable(Integer)).returns(Integer)%7D%0Adef%20foo(x)%0A%20%20y%20%3D%20T.must(x).abs%0A%20%20T.reveal_type(y)%0Aend%0A%0Asig%20%7Bparams(x%3A%20T.nilable(Integer)).returns(T.nilable(Integer))%7D%0Adef%20bar(x)%0A%20%20y%20%3D%20x%26.abs%0A%20%20T.reveal_type(y)%0Aend)

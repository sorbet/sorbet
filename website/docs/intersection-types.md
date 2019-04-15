---
id: intersection-types
title: Intersection Types
---

> TODO: This page is still a fragment. Contributions welcome!

Intersection types can be useful to say, after the fact, that the input must
implement two specific interfaces.

```ruby
# typed: true
extend T::Sig

module I1
  def i1; end
end

module I2
  def i2; end
end

class C
  include I1
  include I2
end

class D
  include I1
  include I2
end


sig {params(x: T.all(I1, I2)).void}
def foo(x)
  x.i1
  x.i2
end

foo(C.new)
foo(D.new)
```

This is useful because we don't have to know ahead of time all the things that
might implement these two interfaces. It also gets around the problem where
we'd have to make a third interface, `I12`, like this:

```ruby
module I12
  include I1
  include I2
end
```

and include this interface into `C` and `D` (because maybe we don't have
control over what interfaces C and D can include).

<a href="https://sorbet.run/#extend%20T%3A%3ASig%0A%0Amodule%20I1%0A%20%20def%20i1%3B%20end%0Aend%0A%0Amodule%20I2%0A%20%20def%20i2%3B%20end%0Aend%0A%0Aclass%20C%0A%20%20include%20I1%0A%20%20include%20I2%0Aend%0A%0Asig%20%7Bparams(x%3A%20T.all(I1%2C%20I2)).void%7D%0Adef%20foo(x)%0A%20%20x.i1%0A%20%20x.i2%0Aend%0A%0Afoo(C.new)%0A">→ View on sorbet.run</a>

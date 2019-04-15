---
id: union-types
title: Union Types
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
class A; end
class B; end
class C; end

class Test
  extend T::Sig

  sig {params(x: T.any(A, B)).returns(String)}
  def self.foo(x)
    # (1) Outside the if: either A or B
    T.reveal_type(x) # Revealed type: `T.any(A, B)`

    # (2) Flow-sensitive typing (see next doc) works on union types
    if x.is_a?(A)
      T.reveal_type(x) # Revealed type: `A`
    else
      T.reveal_type(x) # Revealed type: `B`
    end

    # Both A and B have a #to_s method (because they inherit from Object)
    x.to_s
  end
end

Test.foo(A.new)
Test.foo(B.new)

Test.foo(C.new) # error: `C` doesn't match `T.any(A, B)` for argument `x`
```

<a href="https://sorbet.run/#class%20A%3B%20end%0Aclass%20B%3B%20end%0Aclass%20C%3B%20end%0A%0Aclass%20Test%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Bparams(x%3A%20T.any(A%2C%20B)).returns(String)%7D%0A%20%20def%20self.foo(x)%0A%20%20%20%20T.reveal_type(x)%20%23%20Revealed%20type%3A%20%60T.any(A%2C%20B)%60%0A%20%20%20%20x.to_s%0A%20%20end%0Aend%0A%0ATest.foo(A.new)%0ATest.foo(B.new)%0A%0ATest.foo(C.new)%20%23%20error%3A%20%60C%60%20doesn't%20match%20%60T.any(A%2C%20B)%60%20for%20argument%20%60x%60">→ View on sorbet.run</a>

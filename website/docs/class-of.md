---
id: class-of
title: T.class_of
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
T.class_of(Integer)
```

It can be confusing whether you want `MyClass` or `T.class_of(MyClass)`. For
reference, these assertions are true:

```ruby
T.let(5, Integer)

T.let(5.class, T.class_of(Integer))
T.let(Integer, T.class_of(Integer))
```

You can also use modules.
It respects inheritance in the same way that [Class Types](class-types.md) do:

```ruby
extend T::Sig

class Grandparent; end
class Parent < Grandparent; end
class Child < Parent; end

sig {params(x: T.class_of(Parent)).void}
def foo(x); end

foo(Parent) # ok
foo(Child) # ok

foo(Grandparent) # error
```

<a href="https://sorbet.run/#extend%20T%3A%3ASig%0A%0Aclass%20Grandparent%3B%20end%0Aclass%20Parent%20%3C%20Grandparent%3B%20end%0Aclass%20Child%20%3C%20Parent%3B%20end%0A%0Asig%20%7Bparams(x%3A%20T.class_of(Parent)).void%7D%0Adef%20foo(x)%3B%20end%0A%0A%0Afoo(Parent)%20%23%20ok%0Afoo(Child)%20%23%20ok%0A%0Afoo(Grandparent)%20%23%20error">→ View on sorbet.run</a>

Gotcha: module ancestor chains (`include`, `extend`) work in a way many people
don't expect:

```ruby
# typed: true

extend T::Sig

module M
  def self.foo; end
end
class C; include M; end

sig {params(x: T.class_of(M)).void}
def test(x)
  x.foo
end


M.foo                      # ok
M.is_a?(M.singleton_class) # => true
test(M)                    # ok, given above

C.foo                      # not ok: this raises a NoMethodError when run
C.is_a?(M.singleton_class) # => false
test(C)                    # not ok, given above
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0Aextend%20T%3A%3ASig%0A%0Amodule%20M%0A%20%20def%20self.foo%3B%20end%0Aend%0Aclass%20C%3B%20include%20M%3B%20end%0A%0Asig%20%7Bparams(x%3A%20T.class_of(M)).void%7D%0Adef%20test(x)%0A%20%20x.foo%0Aend%0A%0A%0AM.foo%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20ok%0AM.is_a%3F(M.singleton_class)%20%23%20%3D%3E%20true%0Atest(M)%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20ok%2C%20given%20above%0A%0AC.foo%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20not%20ok%3A%20this%20raises%20a%20NoMethodError%20when%20run%0AC.is_a%3F(M.singleton_class)%20%23%20%3D%3E%20false%0Atest(C)%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20not%20ok%2C%20given%20above">→
View on sorbet.run</a>

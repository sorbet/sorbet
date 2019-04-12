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
[→ View on sorbet.run](https://sorbet.run/#extend%20T%3A%3ASig%0A%0Aclass%20Grandparent%3B%20end%0Aclass%20Parent%20%3C%20Grandparent%3B%20end%0Aclass%20Child%20%3C%20Parent%3B%20end%0A%0Asig%20%7Bparams(x%3A%20T.class_of(Parent)).void%7D%0Adef%20foo(x)%3B%20end%0A%0A%0Afoo(Parent)%20%23%20ok%0Afoo(Child)%20%23%20ok%0A%0Afoo(Grandparent)%20%23%20error)

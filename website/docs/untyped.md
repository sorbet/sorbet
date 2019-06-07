---
id: untyped
title: T.untyped
---

```ruby
T.untyped
```

This indicates a value whose type is statically unknown and will allow all values. The static typechecker will permit
any operation on a `T.untyped` value without complaint.

You can create a `T.untyped` with `T.unsafe`:

```ruby
# typed: true
x = 3
T.reveal_type(x) # Revealed type: Integer(3)
y = T.unsafe(x)
T.reveal_type(y) # Revealed type: T.untyped
```

All arguments to methods without `sig`s have the type `T.untyped`, and the return type is `T.untyped` as well.

```ruby
# typed: true
class C
  extend T::Sig

  sig { params(x: Integer).returns(String) }
  def foo(x)
    T.reveal_type(x) # Revealed type: Integer
    x.to_s
  end

  def bar(x)
    T.reveal_type(x) # Revealed type: T.untyped
    x.to_s
  end
end

T.reveal_type(C.new.foo(3)) # Revealed type: String
T.reveal_type(C.new.bar(3)) # Revealed type: T.untyped
```

You can put `T.untyped` into generics.

```ruby
# typed: true
class C
  extend T::Sig

  sig { params(x: T::Array[T.untyped]).returns(Integer) }
  def foo(x)
    T.reveal_type(x.first) # Revealed type: T.untyped
    x.length
  end
end

T.reveal_type(C.new.foo([1, "bar", false])) # Revealed type: Integer
```

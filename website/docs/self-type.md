---
id: self-type
title: T.self_type
---

> TODO: This page is still a fragment. Contributions welcome!

> **Warning**: This feature is experimental and has known limitations. It may
> not work as expected or change without notice.

```ruby
T.self_type
```

This type can be used in return types to indicate that calling this method on
will return the same type as the type of the receiver (the receiver is the thing
we call a method on i.e., `x` in `x.foo`). For instance, `#dup` returns
`T.self_type`. No matter what class you call it on, you will get back the same
type.

```ruby
# typed: true

class Parent
  extend T::Sig

  sig {returns(T.self_type)}
  def foo
    self
  end
end

class Child < Parent; end

T.reveal_type(Parent.new.foo) # Revealed type: Parent
T.reveal_type(Child.new.foo) # Revealed type: Child

module Mixin
  extend T::Sig

  sig {returns(T.self_type)}
  def bar
    self
  end
end

class UsesMixin
  extend Mixin
end

T.reveal_type(UsesMixin.bar) # Revealed type: T.class_of(UsesMixin)
```

Certain advanced use cases of `T.self_type` are not supported:

```ruby
class Generic < Parent
  extend T::Generic
  TM = type_member

  sig {returns(Generic[T.self_type])} # error: Only top-level T.self_type is supported
  def bad
    Generic[T.untyped].new
  end
end
```

Note that `T.self_type` declares that the type should be **exactly** what the
receiver of the method is. It is **not** useful for declaring the type of
factory methods. For these types of methods, use
[`T.attached_class`](attached-class.md) instead:

```ruby
class Parent
  # sig {returns(T.self_type)} # WRONG
  sig {returns(T.attached_class)}
  def self.make
    new
  end
end

class Child < Parent; end

T.reveal_type(Parent.make) # => `Parent`
T.reveal_type(Child.make)  # => `Child`
```

If the above example was declared using `T.self_type`, Sorbet would expect to
see the `make` method return a value of type `T.class_of(Parent)`, but `new`
returns a value of type `Parent` (an instance of the class, not the class object
itself).

---
id: attached-class
title: T.attached_class
---

`T.attached_class` can be used to refer to the type of instances from a
singleton-context, in a way that is friendly to inheritance.

```ruby
class LoadableData
  extend T::Sig

  sig {params(id: Integer).returns(T.attached_class)}
  def self.load_by_id(id)
    new
  end
end

class Article < LoadableData; end
class User < LoadableData; end
class Administrator < User; end

T.reveal_type(Article.load_by_id(10)) # Article
T.reveal_type(User.load_by_id(15)) # User
T.reveal_type(Administrator.load_by_id(20)) # Administrator
```

## Typing factory methods

Consider the following example of a class hierarchy with a factory method:

```ruby
class Parent
  extend T::Sig

  # Correct, but not ideal return type
  sig {returns(Parent)}
  def self.make
    new
  end
end

class Child < Parent
  sig {void}
  def say_hi
    puts "hi"
  end
end

T.reveal_type(Parent.make) # Parent
T.reveal_type(Child.make) # Parent

Child.make.say_hi # Error: say_hi doesn't exist on Parent
```

`Parent` defines a single factory method `self.make`, which will create an
instance of the `Parent` class. When this method is called off of the `Parent`
class, it returns a new instance of type `Parent`. When it is called on `Child`
it creates an instance of `Child` but the signature of `Parent.make` will cause
the return value to be up-cast to `Parent`. This is fine if it's convenient to
use constructed `Child` values as though they were `Parent`, but becomes
problematic if we need to call methods that are unique to `Child` instances,
like in the case of `Child.make.say_hi`.

This is where `T.attached_class` comes in: changing the return type of
`Parent.make` to `T.attached_class` indicates that it returns instances of the
class the singleton method was called on. When called from `Parent`, it will
return values of type `Parent`, and when called from `Child` it will return
values of type `Child`. This resolves the type errors in the previous example
with `Child.make.say_hi`,

```ruby
class Parent
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.make
    new
  end
end

class Child < Parent
  sig {void}
  def say_hi
    puts "hi"
  end
end

T.reveal_type(Parent.make) # Parent
T.reveal_type(Child.make) # Child

Child.make.say_hi # No error now, as Child.make has the type Child
```

## `T.attached_class` as an argument?

Using `T.attached_class` as the type of parameters is an error in sorbet, and
allowing it would cause soundness issues. To see why, let's take a look at an
example:

```ruby
class Parent
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.make
    new
  end

  sig {params(x: T.attached_class).void} # A bad type definition for `x`
  def self.consume(x)
    puts "consumed"
  end
end

class Child < Parent
  extend T::Sig

  sig {void}
  def say_hi
    puts "hi"
  end

  sig {params(x: T.attached_class).void} # A bad type definition for `x`
  def self.consume(x)
    x.say_hi
  end
end

Parent.consume(Parent.new)
Child.consume(Parent.new) # We would like this to be an error
```

However, problems arise when you begin passing around singleton classes. Imagine
that you write the method below, that accepts arguments of type
`T.class_of(Parent)`.

```ruby
class A
  extend T::Sig

  sig {params(cls: T.class_of(Parent)).void}
  def self.consume_parent(cls)
    cls.consume(Parent.make)
  end
end
```

For the call to `cls.consume` in the body of `A.consume_parent`, we are always
passing an argument whose type is `Parent`. Let's walk through two different
calls to `A.consume_parent`:

- `A.consume_parent(Parent)`
  - `Parent` has type `T.class_of(Parent)`, and is acceptable for `cls`
  - `Parent.consume` accepts values of type `Parent` (via `T.attached_class`)
- `A.consume_parent(Child)`
  - `Child` has type `T.class_of(Child)`, which is a subtype of
    `T.class_of(Parent)` and is acceptable for `cls`
  - `Child.consume` accepts arguments of type `Child` (via `T.attached_class`),
    and thus will raise an error when `say_hi` is called on the instance
    produced by `Parent.make`

As these two cases show, Sorbet can't know whether the body of
`A.consume_parent` type checks or not without knowledge about what type `cls` is
at runtime. Because of this problem, `T.attached_class` is only allowed to show
up in the `returns` part of a signature, and if it does show up in the `params`
of a signature, you'll get an error:

```ruby
class Parent
  extend T::Sig

  sig {params(x: T.attached_class).void}
            # ^: error: type_template <AttachedClass> was defined as :out but is
            # used in an :in context
  def self.problem(x); end
end
```

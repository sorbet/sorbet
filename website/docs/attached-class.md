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

At this point, it's natural to think about writing methods that accept arguments
that take values of type `T.attached_class`; you would get more specific type
information about the arguments when calling from a sub-class, so why not?
Consider this example:

```ruby
class Parent
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.make
    new
  end

  sig {params(x: T.attached_class).void}
  def self.consume(x)
  end
end

class Child < Parent; end

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

When we pass in `Parent` as the argument, everything is fine: `Parent.make`
returns a value of type `Parent`, and `cls.consume` expects an argument of type
`Parent`. However when we pass `Child` as an argument, problems arise:
`Parent.make` still makes a value of type `Parent`, and since the only thing we
know about `cls` is that it is a subtype of `T.class_of(Parent)`, we are forced
to assume that its `T.attached_class` will be `Parent`. The result of this, is
that the signature for `Child.consume` is violated, as it expects an argument
whose type is a subtype of `Child`, and `Parent` doesn't satisfy that condition.

Because of this situation, `T.attached_class` is only allowed to show up in the
`returns` part of a signature, and if it does show up in the `params` of a
signature, you'll get an error:

```ruby
class Parent
  extend T::Sig

  sig {params(x: T.attached_class).void}
            # ^: error: type_template <AttachedClass> was defined as :out but is
            # used in an :in context
  def self.problem(x); end
end
```

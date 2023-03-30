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

Problems arise when you begin passing around singleton classes. Imagine that you
write the method below, that accepts arguments of type `T.class_of(Parent)`.

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
            # ^: T.attached_class may only be used in an :out context, like returns
  def self.problem(x); end
end
```

As a final note: none of these problems would happen if `consume` were private:

- The bad call to `Child.consume(Parent.new)` would not be allowed, because
  `consume` would be private.
- The bad call to `A.consume_parent(Child)` would not be allowed because the
  body of `consume_parent` contains `cls.consume`, which is a non-private call
  to a private method.

As such, Sorbet allows `T.attached_class` to appear in input (`:in`) positions
of private methods.

## `T.attached_class` common problems

One common problem people encounter when using `T.attached_class` looks
something like this:

```ruby
# typed: true

class Parent
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.make
    Parent.new # (1) Sorbet reports an error here
  end
end

class Child < Parent; end

T.reveal_type(Child.make) # Revealed type: `Child`
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Parent%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20sig%20%7Breturns%28T.attached_class%29%7D%0A%20%20def%20self.make%0A%20%20%20%20Parent.new%0A%20%20end%0Aend%0A%0Aclass%20Child%20%3C%20Parent%3B%20end%0A%0AT.reveal_type%28Child.make%29%20%23%20reveals%20Child)

In this example, the program attempts to return `Parent.new` from `self.make`,
instead of just `self.new`. There is a key difference: `Parent.new` will
**always** make an instance of `Parent`, while `self.new` will make an instance
of the particular subclass that `self.make` was called on.

For this reason, Sorbet has to report an error at comment `(1)` above. If we
consider the `Child` class that inherits from `Parent`, the `T.attached_class`
type suggests to the caller that `Child.make` will return a `Child` instance
(the `T.reveal_type` confirms this).

Since `Parent.new` is not an instance of `Child` or any other potential
subclasses of `Parent`, Sorbet must reject the code at `(1)`.

## `has_attached_class!`: `T.attached_class` in module instance methods

Some modules are only ever eventually mixed into a **class** with `extend` (not
`include`), meaning that any methods that module defines will eventually be
called like singleton class methods.

These modules usually want to be able to call `new` to instantiate an instance
of the class that the module is `extend`'d into. That's a problem because
normally constructor methods like that would have a return type of
`T.attached_class`, but instance methods cannot mention `T.attached_class`.

To allow instance methods in such modules to use `T.attached_class`, Sorbet
provides the `has_attached_class!` annotation:

```ruby
module FinderMethods
  extend T::Sig
  extend T::Generic
  abstract!

  has_attached_class!

  sig {abstract.returns(T.attached_class)}
  def new; end

  sig {params(id: String).returns(T.attached_class)}
  def find(id)
    self.new
  end
end

class ParentModel
  extend T::Sig
  extend FinderMethods
end

class ChildModel < ParentModel
end

parent = ParentModel.find('pa_123')
T.reveal_type(parent) # => `ParentModel`
child = ChildModel.find('ch_123')
T.reveal_type(child)  # => `ChildModel`
```

Some things to note:

- The `has_attached_class!` method is exposed as a method in `T::Generic`,
  because using `has_attached_class!` implicitly makes the module into a
  [generic module](generics.md). This is why modules are not allowed to use
  `T.attached_class` by default. More on this in a moment.

- We've declared `new` as an abstract method. This method is automatically
  detected to be implemented when `extend`'d into a class (because all classes
  inherit a concrete `self.new` method).

  This abstract `new` method allows calling `new` in the `find` method. If this
  `find` method had been in an [RBI file](rbi.md) (not in a source file), then
  the abstract `new` declaration would not have been required, because there
  would be no method bodies to type check.

- The `FinderMethods` module is `extended` into `ParentModel`. Had this been
  `include FinderMethods`, Sorbet would have reported an error saying that
  `FinderMethods` can only be extended into classes.

- The two calls to `find` at the bottom of the snippet reveal that `find`'s type
  changes based on the type of the method call's receiver. Basically: `find` on
  a `ChildModel` will be a `ChildModel`.

### Generics and `has_attached_class!`

We mentioned above that using `has_attached_class!` in a module makes the module
into a generic module. The mental model is to think of `has_attached_class!` as
syntactic sugar for putting a `type_member` with an unknown name into the
module. This `type_member`, having no explicit name, can then be referenced
using `T.attached_class`.

What this means is that it's possible to abstract over the attached class of an
`has_attached_class!` module, the same as any other generic interface:

```ruby
sig do
  type_parameters(:U)
    .params(
      findable: FinderMethods[T.type_parameter(:U)]
      id: String
    )
    .returns(T.type_parameter(:U))
end
def find_and_log(findable, id)
  instance = findable.find(id)
  puts("Found #{instance}")
  instance
end

parent = find_and_log(ParentModel, 'pa_123')
T.reveal_type(parent) # => `ParentModel`
child = find_and_log(ChildModel, 'pa_123')
T.reveal_type(child) # => `ChildModel`
```

Note how we've annotated the `findable` parameter as `FinderMethods[...]`,
indicating that we're using the `FinderMethods` module generically. In fact,
supplying a type argument to the `FinderMethods` is now **required**: it's not
possible to reference `FinderMethods` in a type position without providing a
type annotation. If you truly must ignore this, supply a type argument like
`BasicObject` or `T.untyped` (or accept the autocorrect on the error, which will
insert `T.untyped` by default).

As a generic, `has_attached_class!` takes the same arguments that `type_member`
takes for things like variance and bounds:

```ruby
# Declares a covariant type member:
has_attached_class!(:out)

# Places a bound on the type member:
has_attached_class! { {upper: SomeInterface} }

# Altogether:
has_attached_class!(:out) { {upper: SomeInterface} }
```

(Note that this type member cannot be declared contravariant, as that would make
it impossible to mix this module into a class.)

> Note: you may also find this external blog post useful, which discusses
> similar topics:
>
> [Typing klass.new in Ruby with Sorbet →](https://blog.jez.io/typing-klass-new/)

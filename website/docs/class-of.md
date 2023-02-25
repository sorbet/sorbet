---
id: class-of
title: Types for Class Objects via T.class_of
sidebar_label: T.class_of
---

Classes are also values in Ruby. Sorbet uses `T.class_of(...)` to describe the
types of those class objects.

```ruby
T.class_of(Integer)
```

The difference between `MyClass` and `T.class_of(MyClass)` can be confusing.
Here are some examples to make it less confusing:

| These expressions... | ...have these types   |
| -------------------- | --------------------- |
| `0`, `1`, `2 + 2`    | `Integer`             |
| `Integer`            | `T.class_of(Integer)` |
| `42.class`           | `T.class_of(Integer)` |

Here's a playground link to confirm these types:

```ruby
# typed: true
T.let(0, Integer)
T.let(1, Integer)
T.let(2 + 2, Integer)

T.let(Integer, T.class_of(Integer))
T.let(42.class, T.class_of(Integer))
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0AT.let(0%2C%20Integer)%0AT.let(1%2C%20Integer)%0AT.let(2%20%2B%202%2C%20Integer)%0A%0AT.let(Integer%2C%20T.class_of(Integer))%0AT.let(42.class%2C%20T.class_of(Integer))">
  → View on sorbet.run
</a>

## `T.class_of` and inheritance

As with [Class Types](class-types.md#inheritance), `T.class_of` types work with
inheritance:

```ruby
# typed: true
extend T::Sig

class Grandparent; end
class Parent < Grandparent; end
class Child < Parent; end

sig {params(x: T.class_of(Parent)).void}
def example(x); end

example(Grandparent)   # error
example(Parent)        # ok
example(Child)         # ok
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20Grandparent%3B%20end%0Aclass%20Parent%20%3C%20Grandparent%3B%20end%0Aclass%20Child%20%3C%20Parent%3B%20end%0A%0Asig%20%7Bparams(x%3A%20T.class_of(Parent)).void%7D%0Adef%20example(x)%3B%20end%0A%0Aexample(Grandparent)%20%20%20%23%20error%0Aexample(Parent)%20%20%20%20%20%20%20%20%23%20ok%0Aexample(Child)%20%20%20%20%20%20%20%20%20%23%20ok">
  → View on sorbet.run
</a>

The most surprising feature of `T.class_of` comes from not understanding
inheritance in Ruby, especially with `include` or `extend` plus modules.

See below for a common gotcha.

## `T.class_of` and modules

**TL;DR**: `T.class_of` has some unintuitive behavior with modules (as opposed
to classes). Consider either using an abstract class or using
`T.all(Class, MyInterface::ClassMethods)` instead of `T.class_of(MyInterface)`.

To showcase the problem and solutions, let’s walk through a running example. The
full code for this example is available here:

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20Module%3B%20include%20T%3A%3ASig%3B%20end%0A%0Amodule%20MyInterface%0A%20%20extend%20T%3A%3AHelpers%0A%0A%20%20def%20some_instance_method%3B%20end%0A%0A%20%20module%20ClassMethods%0A%20%20%20%20def%20some_class_method%3B%20end%0A%20%20end%0A%20%20mixes_in_class_methods(ClassMethods)%0Aend%0A%0Aclass%20MyClass%0A%20%20include%20MyInterface%0Aend%0A%0Asig%20%7Bparams(x%3A%20T.class_of(MyClass)).void%7D%0Adef%20example1(x)%0A%20%20x.new.some_instance_method%20%20%23%20ok%0A%20%20x.some_class_method%20%20%20%20%20%20%20%20%20%23%20ok%0Aend%0A%0Aexample1(MyClass)%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20ok%0A%0Asig%20%7Bparams(x%3A%20T.class_of(MyInterface)).void%7D%0Adef%20example2(x)%0A%20%20x.new.some_instance_method%20%20%23%20error%3A%20%60new%60%20does%20not%20exist%0A%20%20x.some_class_method%20%20%20%20%20%20%20%20%20%23%20error%3A%20%60some_class_method%60%20does%20not%20exist%0Aend%0A%0Aexample2(MyClass)%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20error%3A%20Expected%20%60T.class_of(MyInterface)%60%20but%20found%20%60T.class_of(MyClass)%60%0A%0Asig%20%7Bparams(x%3A%20T.all(Class%2C%20MyInterface%3A%3AClassMethods)).void%7D%0Adef%20example3(x)%0A%20%20x.new.some_instance_method%20%20%23%20error%3A%20%60some_instance_method%60%20does%20not%20exist%0A%20%20x.some_class_method%20%20%20%20%20%20%20%20%20%23%20ok%0Aend%0A%0Aexample3(MyClass)%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20ok">
  → View on sorbet.run
</a>

Suppose we have some code like this:

```ruby
class MyClass
  def some_instance_method; end
  def self.some_class_method; end
end

sig {params(x: T.class_of(MyClass)).void}
def example1(x)
  x.new.some_instance_method  # ok
  x.some_class_method         # ok
end

example1(MyClass)             # ok
```

`MyClass` declares a class which has an instance method and a class method. The
`T.class_of(MyClass)` annotation allows `example1` to call both those methods.
None of this is too surprising.

Now imagine that we have a lot of these classes and we want to factor out an
interface. The straightforward way to do this uses
[`mixes_in_class_methods`](abstract#interfaces-and-the-included-hook), like
this:

```ruby
module MyInterface
  extend T::Helpers

  def some_instance_method; end

  module ClassMethods
    def some_class_method; end
  end
  mixes_in_class_methods(ClassMethods)
end

class MyClass
  include MyInterface
end
```

This will make `some_instance_method` and `some_class_method` available on
`MyClass`, just like before. But if we try to replace `T.class_of(MyClass)` with
`T.class_of(MyInterface)`, it doesn’t work:

```ruby
sig {params(x: T.class_of(MyInterface)).void}  # ← sig has changed
def example2(x)
  x.new.some_instance_method  # error: `new` does not exist
  x.some_class_method         # error: `some_class_method` does not exist
end

example2(MyClass)             # error: Expected `T.class_of(MyInterface)`
                              #        but found `T.class_of(MyClass)`
```

**These errors are correct**, and we can verify them in the Ruby REPL. First,
let's explain the error on the last line above:

```
❯ MyClass.singleton_class.ancestors
=> [#<Class:MyClass>, MyInterface::ClassMethods, #<Class:Object>, T::Private::Methods::MethodHooks, #<Class:BasicObject>, Class, Module, T::Sig, Object, Kernel, BasicObject]
```

The first two ancestors of the `MyClass` object are itself and
`MyInterface::ClassMethods`. But notably, `#<Class:MyInterface>` **does not**
appear in this list, so Sorbet is correct to say that `MyClass` does not have
type `T.class_of(MyInterface)`. This is because neither `include` nor `extend`
in Ruby will cause `#<Class:MyInterface>` to appear in any ancestors list.

Next, let's explain the other two errors:

```
❯ MyInterface.singleton_class.ancestors
=> [#<Class:MyInterface>, T::Private::MixesInClassMethods, T::Helpers, Module, T::Sig, Object, Kernel, BasicObject]
```

For the `MyInterface` class object, we see that its only ancestor is itself
(ignoring common ancestors like `Object`). Notably, **none** of the classes in
this list define either a method called `new` (because `Class` is not there) nor
`some_class_method` (because `MyInterface::ClassMethods` is not there).

While these errors are technically correct, **we want to be able to type this
code**. There are two options:

1.  Use an abstract class instead of an interface.

    Sometimes this is not possible, because the class in question already has a
    superclass that can't be changed. However, if this option is available, it's
    likely the most straightforward. If we change `MyInterface` to
    `MyAbstractClass`, all our problems vanish.

2.  Use `T.all(Class, MyInterface::ClassMethods)`.

    For our example this is only a partial solution, but in many cases it is
    good enough.

Specifically, option (2) looks like this:

```ruby
sig {params(x: T.all(Class, MyInterface::ClassMethods)).void}
def example3(x)
  x.new.some_instance_method  # error: `some_instance_method` does not exist
  x.some_class_method         # ok
end

example3(MyClass)             # ok
```

We’re down to only one error now. The error is still technically correct: since
we’re using `Class` instead of `T.class_of(...)`, Sorbet has no way to know what
the instance type created by `x.new` will be (it could be anything), so it
treats the type as `Object`, causing `some_instance_method` to not be found.
However, both the top-level call site to `example3` and the call to
`x.some_class_method` now typecheck successfully. In cases where we don't
actually need to use instance methods from `MyInterface`, this may be an
acceptable workaround.

<!-- TODO(jez) Update this doc -->
<!-- TODO(jez) Be sure to include guidance on when to use `T.class_of` vs `T::Class` -->

> A future feature of Sorbet might be able to improve this workaround. See
> https://github.com/sorbet/sorbet/issues/62.

---
id: class-of
title: Types for Class Objects via T.class_of
sidebar_label: T.class_of
---

Classes are also values in Ruby. Sorbet has two ways to describe the type of
these class objects: `T.class_of(...)` and `T::Class[...]`.

```ruby
# The type to use in most circumstances:
T.class_of(MyClass)

# Another type that has certain specific use cases
# (discussed below)
T::Class[MyClass]
```

Prefer `T.class_of(...)` in most cases: it's simpler and leads to fewer
surprises. `T::Class[...]` is better for some very specific use cases, discussed
below. (These specific cases are less common, which is why we recommend using
`T.class_of` to those who don't yet know which to pick.)

## What is a `T.class_of` type?

`T.class_of` is used to refer to the type of a class object itself, not values
of that class. This difference can be confusing, so here are some examples to
make it less confusing:

| This expression... | ...has this type      |
| ------------------ | --------------------- |
| `0`, `1`, `2 + 2`  | `Integer`             |
| `Integer`          | `T.class_of(Integer)` |
| `42.class`         | `T.class_of(Integer)` |

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

As with plain [Class Types](class-types.md#inheritance), `T.class_of` types
respect inheritance:

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

In this example, the `Child` class object passed to the `example` method on the
last line has type `T.class_of(Child)`. The `example` takes
`T.class_of(Parent)`. When one class inherits another, it's singleton class also
inherits the other class's singleton class:

```ruby
# On the class itself, Child < Parent
Child.ancestors
# => [Child, Parent, Grandparent, Object, Kernel, BasicObject]

# On the singleton class, #<Class:Child> < #<Class:Parent>
Child.singleton_class.ancestors
# => [#<Class:Child>, #<Class:Parent>, #<Class:Grandparent>, #<Class:Object>,
      #<Class:BasicObject>, Class, Module, Object, Kernel, BasicObject]
```

Importantly, this only happens for classes, not modules: the singleton class of
a module is _never_ the ancestor of some other class. See the next section for
more.

## `T.class_of` and modules

Usually when people write `T.class_of(MyInterface)`, what they actually want is
either:

- To rewrite the code to use abstract classes instead of interfaces, and then
  use `T.class_of(MyAbstractClass)`, or
- To use a type like `T.all(T::Class[MyInterface], MyInterface::ClassMethods)`

To showcase why `T.class_of(MyInterface)` is usually a problem and why these two
are better solutions, let's walk through an example. The full code for this
example is available here:

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Aclass%20Module%3B%20include%20T%3A%3ASig%3B%20end%0A%0Amodule%20MyInterface%0A%20%20extend%20T%3A%3AHelpers%0A%0A%20%20def%20some_instance_method%3B%20end%0A%0A%20%20module%20ClassMethods%0A%20%20%20%20def%20some_class_method%3B%20end%0A%20%20end%0A%20%20mixes_in_class_methods%28ClassMethods%29%0Aend%0A%0Aclass%20MyClass%0A%20%20include%20MyInterface%0Aend%0A%0Asig%20%7Bparams%28x%3A%20T.class_of%28MyClass%29%29.void%7D%0Adef%20example1%28x%29%0A%20%20x.new.some_instance_method%20%20%23%20ok%0A%20%20x.some_class_method%20%20%20%20%20%20%20%20%20%23%20ok%0Aend%0A%0Aexample1%28MyClass%29%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20ok%0A%0Asig%20%7Bparams%28x%3A%20T.class_of%28MyInterface%29%29.void%7D%0Adef%20example2%28x%29%0A%20%20x.new.some_instance_method%20%20%23%20error%3A%20%60new%60%20does%20not%20exist%0A%20%20x.some_class_method%20%20%20%20%20%20%20%20%20%23%20error%3A%20%60some_class_method%60%20does%20not%20exist%0Aend%0A%0Aexample2%28MyClass%29%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20error%3A%20Expected%20%60T.class_of%28MyInterface%29%60%20but%20found%20%60T.class_of%28MyClass%29%60%0A%0Asig%20%7Bparams%28x%3A%20T.all%28T%3A%3AClass%5BMyInterface%5D%2C%20MyInterface%3A%3AClassMethods%29%29.void%7D%0Adef%20example3%28x%29%0A%20%20x.new.some_instance_method%20%20%23%20ok%0A%20%20x.some_class_method%20%20%20%20%20%20%20%20%20%23%20ok%0Aend%0A%0Aexample3%28MyClass%29%20%20%20%20%20%20%20%20%20%20%20%20%20%23%20ok">
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
None of this is surprising.

Now imagine that we have a lot of these classes and we want to factor out an
interface. The straightforward way to factor out an interface that defines both
instance and singleton class methods uses
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
`T.class_of(MyInterface)`, it doesn't work:

```ruby
sig {params(x: T.class_of(MyInterface)).void}  # ← sig has changed
def example2(x)
  x.new.some_instance_method  # error: `new` does not exist
  x.some_class_method         # error: `some_class_method` does not exist
end

example2(MyClass)             # error: Expected `T.class_of(MyInterface)`
                              #        but found `T.class_of(MyClass)`
```

**These errors are correct**. Conceptually, `T.class_of(MyInterface)` represents
the type of the `MyInterface` class object _itself_, not "any class object whose
instances implement `MyInterface`." We can verify these errors are correct in
the repl.

First, we can explain the error on the call to `example2` by looking at
ancestors:

```
❯ MyClass.singleton_class.ancestors
=> [#<Class:MyClass>, MyInterface::ClassMethods,
    #<Class:Object>, T::Private::Methods::MethodHooks, #<Class:BasicObject>,
    Class, Module, T::Sig, Object, Kernel, BasicObject]
```

The first two ancestors of the `MyClass` singleton class are itself and
`MyInterface::ClassMethods`. But notably, `#<Class:MyInterface>` **does not**
appear in this list, so Sorbet is correct to say that `MyClass` does not have
type `T.class_of(MyInterface)`. This is because neither `include` nor `extend`
in Ruby will cause `#<Class:MyInterface>` to appear in any ancestors list.

Next, let's explain the other two errors:

```
❯ MyInterface.singleton_class.ancestors
=> [#<Class:MyInterface>,
    T::Private::MixesInClassMethods, T::Helpers, Module, T::Sig, Object,
    Kernel, BasicObject]
```

For the `MyInterface` singleton class, we see that its only ancestor is itself
(ignoring common ancestors like `Object`). Notably, **none** of the classes in
this list define either a method called `new` (because `Class` is not there) nor
`some_class_method` (because `MyInterface::ClassMethods` is not there).

While these errors are technically correct, we want to be able to type this
code. There are two options:

1.  Use an abstract class instead of an interface.

    If this option is available, it's likely the most straightforward. If we
    change `MyInterface` to `MyAbstractClass`, all our problems vanish.
    Sometimes this is not possible, because the class in question already has a
    superclass that can't be changed.

2.  Use `T.all(T::Class[MyInterface], MyInterface::ClassMethods)`.

Specifically, option (2) looks like this:

```ruby
sig {params(x: T.all(T::Class[MyInterface], MyInterface::ClassMethods)).void}
def example3(x)
  x.new.some_instance_method  # ok
  x.some_class_method         # ok
end

example3(MyClass)             # OK
```

We discuss `T::Class` more in the next section. To break down that large type:

- `T.all` is an [Intersection Type](intersection-types.md), which says that `x`
  has both the type `T::Class[MyInterface]` and `MyInterface::ClassMethods`.
  It's allowed to call all the methods defined on those types individually.

- `T::Class[MyInterface]` is a type that represents "any class object which,
  when instantiated, creates instances that at least have type `MyInterface`."
  Other than that, it says nothing about what singleton class methods the class
  object has, which means it only assumes those that are defined on `::Class` in
  the Ruby standard library (basically, just `.new` and `.name`). But Sorbet is
  smart enough to know that objects created by calling `new` have type
  `MyInterface`, and thus that `some_instance_method` exists.

- `MyInterface::ClassMethods`

  This module holds all of the interface's class methods, including
  `some_class_methods`.

## `T::Class` vs `T.class_of`

`T::Class` was designed to model some mismatches between how people think they
can use `T.class_of` and how `T.class_of` actually works. `T::Class` is powered
by Sorbet's support for [generic classes](generics.md), and is therefore a good
choice for writing code that abstracts over over class objects.

What are these mismatches? `T.class_of(...)` is, simply, a type representing the
singleton class of `A`, matching how singleton classes work in Ruby as closely
as possible. However:

- Arbitrary types don't necessarily have singleton classes: for example,
  `T.class_of(T.noreturn)` is not a valid type, and neither is
  `T.class_of(T.any(A, B))`.
- As we saw in the previous section, `T.class_of(MyInterface)` **does not mean**
  "any class object which, when instantiated, creates instances that at least
  have type `MyInterface`."

Sorbet provides `T::Class` to relax these restrictions. Like other
`T::`-prefixed types, this is a [typed wrapper](stdlib-generics.md) for the
`::Class` class defined in the Ruby standard library. It's also a
[generic class](generics.md), which means it can be given an arbitrary type,
instead of only classes. And finally, the generic type parameter on `T::Class`
uses the same internal mechanism as Sorbet's
[`T.attached_class` type](attached-class.md), which represents "an instance of
the current class."

Combined, these features allow `T::Class[...]` to model some common Ruby
patterns. For example:

```ruby
sig do
  type_parameters(:Instance)
    .params(klass: T::Class[T.type_parameter(:Instance)])
    .returns(T.type_parameter(:Instance))
end
def instantiate_class(klass)
  instance = klass.new
  puts("Instantiated: #{instance}")
  instance
end

class A; end
class B; end

# converts T.class_of(A) -> A
a = instantiate_class(A)

# converts T.class_of(B) -> B
b = instantiate_class(B)
```

The example above uses [a generic method](generics.md#generic-methods) to take
any class object, instantiate it, and understand that the return value's type is
the [attached class](attached-class.md) of the class object that was passed in.
Calling `instantiate_class(A)` takes a value of type `T.class_of(A)` and
produces a value of type `A`. `T::Class[T.type_parameter(:U)]` is a type we can
actually write because `T::Class` is a full-fledged generic class. By contrast,
we can't write `T.class_of(T.type_parameter(:U))`,because an arbitrary type like
`T.type_parameter(:U)` might not have a singleton class.

Another example:

```ruby
module AbstractCommand
  extend T::Helpers
  interface!
  sig {abstract.void}
  def run; end
end

class MyCommand
  include AbstractCommand

  sig {override.void}
  def run; puts("Hello, world!"); end
end

sig {params(command_klass: T::Class[AbstractCommand]).void}
def run_command(command_klass)
  # (1) Instantiate some command class
  command = command_klass.new
  T.reveal_type(command) # => AbstractCommand
  # (2) Run the command
  command.run
end

run_command(MyCommand)
```

In this example, we use `T::Class` to place a constraint on the class object's
attached class. The `run_command` method takes class objects, but only those
whose attached classes implement the `AbstractCommand` interface. At point (1)
we use the class object to instantiate `command_class`, and Sorbet understands
that the resulting value has type `AbstractCommand`. This allows point (2) to
type check, because Sorbet will know that the `.run` method exists.

### Why have both `T.class_of` and `T::Class`?

There are some things that are only possible to represent with `T.class_of`, and
some things that are only possible to represent with `T::Class`.

- `T::Class` is generic in its attached class. It can be applied to an arbitrary
  type, which means that things like `T::Class[T.any(A, B)]` and
  `T::Class[MyInterface]` work.

  By contrast, it's simply a syntax error to write `T.class_of(T.any(A, B))`
  (because this doesn't resolve to a single attached class), and
  `T.class_of(MyInterface)` means something different from what people might
  otherwise expect it to mean.

- `T.class_of` knows what methods are on the singleton class of a class. By
  contrast, given this:

  ```ruby
  class MyClass
    def self.foo; end
  end
  ```

  The type `T::Class[MyClass]` doesn't represent what singleton class methods
  exist on that class object, only that the associated instance type is. But
  `T.class_of(MyClass)` represents both what singleton class methods exist, and
  also that creating an instance of this class will have type `MyClass`.

So these two types are similar, but each has functionality unique to itself.

The fact that the names are so similar is an unfortunate consequence of history.
It might have been better to use syntax like `T.singleton_class(A)` (or maybe
even `A.singleton_class`) if we could have anticipated that we would eventually
want to build `T::Class` one day.

### `T::Class` vs `Class`

In old versions of Sorbet, the `::Class` class in the Ruby standard library was
not generic. In versions of Sorbet that support `T::Class`, `::Class` became
generic. Sorbet requires that generic classes in type annotations not be
bare--they must be applied to a type argument.

For more information, see
[this section in the docs](stdlib-generics.md#generic-class-without-type-arguments).

The difference between `T::Class` and `Class` is the same as the difference
between `T::Array` and `Array`. `T::Class` and `Class` represent the same class
definition in the standard library, but `T::Class` allows passing type arguments
to the generic type parameters defined in `Class`. This error is only reported
at [`# typed: strict` or higher](static.md). At lower levels, Sorbet implicitly
assumes that a bare type annotation like `Class` is the same as
`T::Class[T.anything]`. (See [`T.anything`](anything.md).)

Feel free to replace `Class` with `T::Class[T.anything]` in type annotations
where nothing is known about the class object. If there's an obvious more
specific type, feel free to narrow `T.anything` to whatever the more specific
type is.

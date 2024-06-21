---
id: abstract
title: Abstract Classes and Interfaces
sidebar_label: Abstract Classes & Interfaces
---

Sorbet supports abstract classes, abstract methods, and interfaces. Abstract
methods ensure that a particular method gets implemented anywhere the class or
module is inherited, included, or extended. An abstract class or module is one
that contains one or more abstract methods. An interface is a class or module
that must have only abstract methods.

Keep in mind:

- `abstract!` can be used to prevent a class from being instantiated.
- Both `abstract!` and `interface!` allow the class or module to have `abstract`
  methods.
- Mix in a module (via `include` or `extend`) to declare that a class implements
  an interface.

> **Note**: Most of the abstract and override checks are implemented statically,
> but some are still only implemented at runtime, most notably variance checks.

## Creating an abstract method

To create an abstract method:

1.  Add `extend T::Helpers` to the class or module (in addition to
    `extend T::Sig`).
1.  Add `abstract!` or `interface!` to the top of the class or module. (_All_
    methods must be abstract to use `interface!`.)
1.  Add a `sig` with `abstract` to any methods that should be abstract, and thus
    implemented by a child.
1.  Declare the method on a single line with an empty body.

```ruby
module Runnable
  extend T::Sig
  extend T::Helpers                                    # (1)
  interface!                                           # (2)

  sig {abstract.params(args: T::Array[String]).void}   # (3)
  def main(args); end                                  # (4)
end
```

## Implementing an abstract method

To implement an abstract method, define the method in the implementing class or
module with an identical signature as the parent, except replacing `abstract`
with `override`.

```ruby
class HelloWorld
  extend T::Sig
  include Runnable

  # This implements the abstract `main` method from our Runnable module:
  sig {override.params(args: T::Array[String]).void}
  def main(args)
    puts 'Hello, world!'
  end
end
```

## Additional Requirements

There are some additional stipulations on the use of `abstract!` and
`interface!`:

- All methods in a module marked as `interface!` must have signatures, and must
  be marked `abstract`.
  - **Note**: this applies to all methods defined within the module, as well as
    any that are included from another module
- A module marked `interface!` can't have `private` or `protected` methods.
- Any method marked `abstract` must have no body. `sorbet-runtime` will take
  care to raise an exception if an abstract method is called at runtime.
- Classes without `abstract!` or `interface!` must implement all `abstract`
  methods from their parents.
- `extend MyAbstractModule` works just like `include MyAbstractModule`, but for
  singleton methods.
- `abstract!` classes cannot be instantiated (will raise at runtime).

## `overridable`: Providing default implementations of methods

Certain abstract classes or interfaces want to provide methods that provide a
reasonable default implementation of a method, allowing individual children to
override the method with a more specific implementation.

This is done with `overridable`:

```ruby
module Countable
  extend T::Helpers

  # 1: `abstract!` instead of `interface!`
  abstract!

  sig { abstract.returns(T.nilable(Integer)) }
  def to_count; end

  # 2: Use `overridable` to provide default implementation of `to_count!`
  sig { overridable.returns(Integer) }
  def to_count!
    T.must(self.to_count)
  end
end
```

As the example shows, there are two main steps:

1. If the module is not already `abstract!` (i.e., if it's an `interface!`),
   change it to use `abstract!`. Modules declared with `interface!` are
   constrained to _only_ have abstract methods, which prevents adding methods
   with a default implementation.

2. Use `overridable` to declare the default implementation of a method. Using
   `overridable` opts the method into static
   [override checking](override-checking.md), which will ensure that children
   define a type-compatible override.

Note: if you want to provide functionality in an abstract class or module that
**must not** be possible to override in a child, use a [final method](final.md).

## Letting abstract methods be implemented via inheritance

Sorbet allows abstract methods in modules to be implemented by an **ancestor**
of the class or module they're eventually mixed into. Consider this example:

```ruby
class Parent
  sig { void }
  def foo = puts 'Hello!'
end

module IFoo
  extend T::Helpers
  abstract!

  sig { abstract.void }
  def foo; end
end

class Child < Parent # ✅ okay
  include IFoo
end

class NotAParent # ❌ Missing definition for `foo`
  include IFoo
end
```

Breaking down this example:

- The `IFoo` module declares a single, abstract `foo` method. All classes that
  include this module must either be marked abstract or define this method.

- The `Parent` method does **not** depend on `IFoo`, **but** does happen to
  define a method called `foo`.

- Both `Child` and `NotAParent` have `include IFoo`, but neither define a `foo`
  method.

- Despite this: only `NotAParent` has an error saying that a concrete
  implementation of `foo` is missing. Sorbet allows the `foo` method to be
  implemented in `Child` because it inherits a `foo` method from `Parent`.

### Approximating duck types

This technique is particularly useful as a way to approximate "duck typing,"
where you depend on "anything type, so long as it has this method."

For example:

```ruby
module ShortName
  extend T::Helpers
  abstract!
  sig { abstract.returns(T.nilable(String)) }
  def name; end

  sig { returns(T.nilable(String)) }
  def short_name
    self.name&.split('::')&.last
  end
end
```

This module provides a `short_name` method (which computes the "short name" of a
something like a `Module` by splitting the full name into `::`-delimited tokens
and returning the last one. Like `C` for `module A::B::C`).

The module's implementation depends on the `name` method existing. If we don't
declare it as an `abstract` method, Sorbet reports an error saying "Method
`name` does not exist," which is true--there's no guarantee someone mixes this
module into a context where `name` is defined.

But by declaring `name` as an abstract method, Sorbet will check this property.
In particular, this has the effect of catching someone who accidentally uses
`include` instead of `extend` when mixing this module into a class:

```ruby
class A
  include ShortName # ❌ error: Must define abstract method `name`
end

class B
  extend ShortName # ✅
end
```

This technique is particularly effective when it's not possible to refactor some
upstream dependency's code to expose an explicit interface. The `Module` class
in the Ruby stdlib doesn't have some sort of public `INameable` interface with
the `name` method. A handful of database model classes in an application might
share a set of related fields, without explicitly implementing some interface.
And yet, using this technique Sorbet allows writing modules which depend on
those implicit interfaces.

This technique is also quite flexible: the `ShortName` module can be used in
**any** context where a `name` method is available. So for example, if you had
some `T::Struct` that stores a `name`, this `ShortName` mixin could also be
used:

```ruby
class C < T::Struct
  include ShortName
  prop :name, String
end

C.new(name: "Some::Long::Namespace").short_name # => "Namespace"
```

### Consequences for runtime signature checking

Letting abstract methods be implemented by inherited methods relies on the fact
that method signatures are [checked at runtime](runtime.md). To explain why this
feature requires runtime support, let's look at the resolved ancestors of
`Child`:

```
irb> Child.ancestors
=> [Child, IFoo, Parent, <...>]
```

This shows that Ruby resolves a call like `child.foo` by first checking whether
`Child` defines `foo`, then whether `IFoo` defines `foo`, and then finally
whether `Parent` does. Since it looks in `IFoo` **before** `Parent`, Ruby
actually calls the `IFoo#foo` method. But this method would normally have an
empty method body—it's abstract!

So at runtime, the `sig` method replaces the implementation of `foo` with a
method that does something like this:

```ruby
def foo
  if defined?(super)
    super
  else
    raise NotImplementedError.new("Call to unimplemented abstract method")
  end
end
```

This allows `IFoo#foo` to dispatch up the ancestor chain, letting `child.foo`
result in a call to `Parent#foo`.

If runtime signature checking is disabled, a call like `child.foo` will silently
produce `nil` instead of calling the appropriate method.

## Abstract singleton methods

`abstract` singleton methods on a module are not allowed, as there's no way to
implement these methods.

```ruby
module M
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def self.foo; end
end

M.foo # error: `M.foo` can never be implemented
```

Abstract singleton methods on a class **are** allowed, but are unsound (i.e.,
they can lead to runtime, type-related exceptions like `TypeError` and
`NameError` even when there is no `T.untyped` involved):

```ruby
class AbstractParent
  abstract!
  sig { abstract.void } # ❌ BAD: abstract singleton class method!
  def self.foo; end
end

class ConcreteChild < AbstractParent
  sig { override.void }
  def self.foo = puts("hello!")
end

sig { params(klass: T.class_of(AbstractParent)).void }
def example(klass)
  klass.foo
end

example(ConcreteChild)  # ✅ okay
example(AbstractParent) # static:  ✅ no errors
                        # runtime: 💥 call to abstract method foo
```

For more information, see this blog post:

[Abstract singleton class methods are an abomination →](https://blog.jez.io/abstract-singleton-methods)

The blog post above discusses the problem and three alternatives to avoid using
abstract singleton class methods. To summarize:

1.  Declare an interface or abstract module with abstract instance methods, and
    `extend` that module onto a class.

1.  Use the above approach, but with
    [`mixes_in_class_methods`](#interfaces-and-the-included-hook), discussed
    below.

1.  Make the method `overridable` instead of `abstract`, effectively giving the
    method a default implementation.

There are also some runtime escape hatches to work around this problem. See
[Runtime reflection on abstract classes](#runtime-reflection-on-abstract-classes)
below.

## Interfaces and the `included` hook

A somewhat common pattern in Ruby is to use an `included` hook to mix class
methods from a module onto the including class:

```ruby
module M
  module ClassMethods
    def foo
      self.bar
    end
  end

  def self.included(other)
    other.extend(ClassMethods)
  end

end

class A
  include M
end

# runtime error as `bar` is not defined on A
A.bar
```

This is hard to statically analyze, as it involves looking into the body of the
`self.included` method, which might have arbitrary computation. As a compromise,
Sorbet provides a new construct: `mixes_in_class_methods`. At runtime, it
behaves as if we'd defined `self.included` like above, but will declare to `srb`
statically what module is being extended.

We can update our previous example to use `mixes_in_class_methods`, which lets
Sorbet catch the runtime error about `bar` not being defined on `A`:

```ruby
# typed: true
module M
  extend T::Helpers
  interface!

  module ClassMethods
    extend T::Sig
    extend T::Helpers
    abstract!

    sig {void}
    def foo
      bar
    end

    sig {abstract.void}
    def bar; end
  end

  mixes_in_class_methods(ClassMethods)
end

class A # error: Missing definition for abstract method
  include M

  extend T::Sig

  sig {override.void}
  def self.bar; end
end

# Sorbet knows that `foo` is a class method on `A`
A.foo
```

We can also call `mixes_in_class_methods` with multiple modules to mix in more
methods. Some Ruby modules mixin more than one module as class methods when they
are included, and some modules mixin class methods but also include other
modules that mixin in their own class modules. In these cases, you will need to
declare multiple modules in the `mixes_in_class_methods` call or make multiple
`mixes_in_class_methods` calls.

For a more comprehensive resource on how `mixes_in_class_methods` builds on
existing Ruby inheritance features, see this blog post:

[Inheritance in Ruby, in pictures →](https://blog.jez.io/inheritance-in-ruby)

## Runtime reflection on abstract classes

From time to time, it's useful to be able to ask whether a class or module
object is abstract at runtime.

This can be done with

```ruby
sig {params(mod: Module).void}
def example(mod)
  if T::AbstractUtils.abstract_module?(mod)
    puts "#{mod} is abstract"
  else
    puts "#{mod} is concrete"
  end
end
```

Note that in general, having to ask whether a module is abstract is a **code
smell**. There is usually a way to reorganize the code such that calling
`abstract_module?` isn't needed. In particular, this happens most frequently
from the use of modules with
[abstract singleton class methods](#abstract-singleton-methods) (abstract
`self.` methods), and the fix is to stop using abstract singleton class methods.

Here's an example:

```ruby
# typed: true

# --- This is an example of what NOT to do ---

extend T::Sig

class AbstractFoo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def self.example; end
end

class Foo < AbstractFoo
  sig {override.void}
  def self.example
    puts 'Foo#example'
  end
end

sig {params(mod: T.class_of(AbstractFoo)).void}
def calls_example_bad(mod)
  # even though there are no errors,
  # the call to mod.example is NOT always safe!
  # (see comments below)
  mod.example
end

sig {params(mod: T.class_of(AbstractFoo)).void}
def calls_example_okay(mod)
  if !T::AbstractUtils.abstract_module?(mod)
    mod.example
  end
end

calls_example_bad(Foo)         # no errors
calls_example_bad(AbstractFoo) # no static error, BUT raises at runtime!

calls_example_okay(Foo)         # no errors
calls_example_okay(AbstractFoo) # no errors, because of explicit check
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20AbstractFoo%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AHelpers%0A%20%20abstract!%0A%0A%20%20sig%20%7Babstract.void%7D%0A%20%20def%20self.example%3B%20end%0Aend%0A%0Aclass%20Foo%20%3C%20AbstractFoo%0A%20%20sig%20%7Boverride.void%7D%0A%20%20def%20self.example%0A%20%20%20%20puts%20'Foo%23example'%0A%20%20end%0Aend%0A%0Asig%20%7Bparams%28mod%3A%20T.class_of%28AbstractFoo%29%29.void%7D%0Adef%20calls_example_bad%28mod%29%0A%20%20%23%20even%20though%20there%20are%20no%20errors%2C%0A%20%20%23%20the%20call%20to%20mod.example%20is%20NOT%20always%20safe!%0A%20%20%23%20%28see%20comments%20below%29%0A%20%20mod.example%0Aend%0A%0Asig%20%7Bparams%28mod%3A%20T.class_of%28AbstractFoo%29%29.void%7D%0Adef%20calls_example_okay%28mod%29%0A%20%20if%20!T%3A%3AAbstractUtils.abstract_module%3F%28mod%29%0A%20%20%20%20mod.example%0A%20%20end%0Aend%0A%0Acalls_example_bad%28Foo%29%20%20%20%20%20%20%20%20%20%23%20no%20errors%0Acalls_example_bad%28AbstractFoo%29%20%23%20no%20static%20error%2C%20BUT%20raises%20at%20runtime!%0A%0Acalls_example_okay%28Foo%29%20%20%20%20%20%20%20%20%20%23%20no%20errors%0Acalls_example_okay%28AbstractFoo%29%20%23%20no%20errors%2C%20because%20of%20explicit%20check)

In the example above, `calls_example_bad` is bad because `mod.example` is not
always okay to call, despite Sorbet reporting no errors. In particular,
`calls_example_bad(AbstractFoo)` will raise an exception at runtime because
`example` is an abstract method with no implementation.

An okay, but not great, fix for this is to call `abstract_module?` before the
call to `mod.example`, which is demonstrated in `calls_example_okay`.

Most other languages simply do not allow defining abstract singleton class
methods (for example, `static` methods in TypeScript, C++, Java, C#, and more
are not allowed to be abstract). For historical reasons attempting to make
migrating to Sorbet easier in existing Ruby codebases, Sorbet allows abstract
singleton class methods.

A better solution is to make an interface with abstract methods, and `extend`
that interface into a class:

```ruby
# typed: true
extend T::Sig

module IFoo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def example; end
end

class Foo
  extend T::Sig
  extend IFoo
  sig {override.void}
  def self.example
    puts 'Foo#example'
  end
end

sig {params(mod: IFoo).void}
def calls_example_good(mod)
  # call to mod.example is always safe
  mod.example
end


calls_example_good(Foo)  # no errors
calls_example_good(IFoo) # doesn't type check
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Amodule%20IFoo%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AHelpers%0A%20%20abstract!%0A%0A%20%20sig%20%7Babstract.void%7D%0A%20%20def%20example%3B%20end%0Aend%0A%0Aclass%20Foo%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20IFoo%0A%20%20sig%20%7Boverride.void%7D%0A%20%20def%20self.example%0A%20%20%20%20puts%20'Foo%23example'%0A%20%20end%0Aend%0A%0Asig%20%7Bparams%28mod%3A%20IFoo%29.void%7D%0Adef%20calls_example_good%28mod%29%0A%20%20%23%20call%20to%20mod.example%20is%20always%20safe%0A%20%20mod.example%0Aend%0A%0A%0Acalls_example_good%28Foo%29%20%20%23%20no%20errors%0Acalls_example_good%28IFoo%29%20%23%20doesn't%20type%20check)

In this example, unlike before, we have a module `IFoo` with an abstract
**instance** method, instead of a class `AbstractFoo` with an abstract singleton
class method. This module is then `extend`'ed into `class Foo` to implement the
interface.

This fixes all of our problems:

- We no longer need to use `abstract_module?` to check whether `mod` is
  abstract.
- Sorbet statically rejects `calls_example_good(IFoo)` (intuitively: because
  `IFoo.example` is not a method that even exists).

Another benefit is that now we have an explicit interface that can be documented
and implemented by any class, not just subclasses of `AbstractFoo`.

## What's next?

- [Override Checking](override-checking.md)

  Sorbet has more ways to check overriding than just whether an abstract method
  is implemented in a child. See this doc to learn about the ways to declare
  what kinds of overriding should be allowed.

- [Sealed Classes and Modules](sealed.md)

  Abstract classes and interfaces are frequently used with sealed classes to
  recreate a sort of "algebraic data type" in Sorbet.

---
id: class-types
title: Class Types
sidebar_label: Class Types (Integer, String)
---

> **Note**: Class types are used to describe values that are instances of a
> class---these are the most commonly used types. To instead learn about types
> for class objects themselves, see [T.class_of](class-of.md).

Every Ruby class and module doubles as a type in Sorbet. Class types supersede
the notion some other languages have of "primitive" types. For example, `"abc"`
is an instance of the `String` class, and so `"abc"` has type `String`. The same
goes for many other values in Ruby:

| Type       | Example value |
| ---------- | ------------- |
| `String`   | `"abc"`       |
| `Symbol`   | `:abc`        |
| `Integer`  | `42`          |
| `Float`    | `3.14`        |
| `NilClass` | `nil`         |

To reiterate: a class type means "any value which is an instance of this class".
If `x.is_a?(SomeClass)` would return `true` when run, then `x` has type
`SomeClass`.

We can mention class types directly in a [method signature](sigs.md):

```ruby
sig {returns(Integer)}
def age
  25
end

sig {params(x: Float).returns(String)}
def float_to_string(x)
  x.to_s
end
```

## Booleans

One gotcha is that `false` is an instance of `FalseClass`, and `true` is an
instance of `TrueClass`---there is no `Boolean` class in Ruby. So to represent
the type of booleans in Sorbet, the `sorbet-runtime` uses
[type aliases](type-aliases.md) and [union types](union-types.md) to define a
convenient name for "either `true` or `false`": `T::Boolean`.

```ruby
extend T::Sig

sig {params(new_value: T::Boolean).void}
def set_flag(new_value)
  @flag = new_value
  puts "Set value to #{new_value}"
end

set_flag(true)
set_flag(false)
```

## `nil`

Note that the class (and type) of `nil` is `NilClass`.

There's a lot to say about `nil`, so it gets [its own doc](nilable-types.md).

## User-defined class types

Everything we've seen so far has used classes built into Ruby, but it works the
exact same for any classes we define ourselves:

```ruby
extend T::Sig

class MyClass; end

sig {returns(MyClass)}
def foo
  MyClass.new
end
```

## Inheritance

<!-- id="inheritance" is important because other pages link here, so please don't change the section title -->

Ruby is object-oriented, so an instance of a child class is also an instance of
the child class's superclass. To make this more explicit, let's look at an
example:

```ruby
extend T::Sig

# Set up an inheritance relationship between three classes
class GrandParentClass; end
class ParentClass < GrandParentClass; end
class ChildClass < ParentClass; end

# Takes ParentClass or lower, not GrandParentClass
sig {params(x: ParentClass).void}
def foo(x); end

foo(GrandParentClass.new)  # error
foo(ParentClass.new)       # ok
foo(ChildClass.new)        # ok
```

## `Object` vs `BasicObject`

Another note about inheritance in Ruby concerns the distinction between `Object`
and `BasicObject`. `Object` is what classes subclass by default, unless they
explicitly subclass from `BasicObject`. `Object` subclasses from `BasicObject`.
Again, let's make this clear with an example:

```ruby
# Some helper methods to play with
sig {params(x: Object).void}
def takes_object(x); end
sig {params(x: BasicObject).void}
def takes_basic_object(x); end

# The one error is because an instance of BasicObject is not an instance of Object
takes_object(Object.new)              # ok
takes_object(BasicObject.new)         # error
takes_basic_object(Object.new)        # ok
takes_basic_object(BasicObject.new)   # ok

# Some classes to play around with
class ObjectChild; end
class BasicObjectChild < BasicObject; end

# The class that explicitly subclasses `BasicObject`
# can't be given to `takes_object`.
takes_object(ObjectChild.new)              # ok
takes_object(BasicObjectChild.new)         # error
```

## Modules

Modules can be used as "class types" in exactly the same way as classes can. For
a module, the meaning is not "an instance of this class" but "an instance of a
class which `include`s this module". This is compatible with how
`x.is_a?(SomeModule)` works in Ruby. Here's an example:

```ruby
extend T::Sig

module MyModule
  def some_method
    puts 'inside MyModule'
  end
end

class MyClass
  include MyModule
end

sig {params(x: MyModule).void}
def foo(x)
  x.some_method
end

foo(MyClass.new)  # ok; MyClass mixes in MyModule
```

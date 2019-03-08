---
id: class-types
title: Class Types
---

> TODO: This page is still a fragment. Contributions welcome!

Every Ruby class is a valid type. For example:

- `String`
- `Symbol`
- `Integer`

The type of `nil` is `NilClass`. See also: [Nilable Types](nilable-types.md).

There is no `Boolean` type in Ruby. Instead, it has `TrueClass` and
`FalseClass`. For convenience, `sorbet-runtime` uses a [type
alias](type-aliases.md) to define a global `T::Boolean` type:

- `T::Boolean`

Any user-defined class can be a type:

```ruby
extend T::Sig

class MyClass; end

sig {returns(MyClass)}
def foo
  MyClass.new
end
```

A subclass can be used in place of a parent:

```ruby
extend T::Sig

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

You can use modules to mean "any class which includes this module":

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



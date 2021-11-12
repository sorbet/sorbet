---
id: tstruct
title: Typed Structs via T::Struct
sidebar_label: T::Struct
---

> TODO: This page is still a fragment. Contributions welcome!

While Sorbet supports using Hashes and Structs, it isn't always possible to
model them statically.

The `sorbet-runtime` gem ships with an alternative to using Ruby's `Hash` or
`Struct` classes that is more well supported: `T::Struct`.

## Creating a new struct class

To use `T::Struct`, first create a new class that directly inherits from it.
Then, in the body of the class, declare the fields on the struct.

```ruby
# typed: true
require 'sorbet-runtime'

# (1) Create a new class that subclasses `T::Struct`
class MyStruct < T::Struct
  # (2) Declare fields on the struct with the `prop` and `const` DSL
  prop :foo, Integer
  const :bar, T.nilable(String)
  const :quz, Float, default: 0.5
end
```

## Creating and using an instance of a struct

After defining a struct class, create instances of the class with `new` as
usual. Then use the generated getters and setters to access the instance's
fields.

```ruby
# (1) Initialize an instance of the struct
my_struct = MyStruct.new(foo: 3)

# (2) Use the generated field getters and setters
my_struct.foo # => 3
my_struct.bar # => nil
my_struct.quz # => 0.5
my_struct.foo = 4
my_struct.foo # => 4
```

Note that:

- `new` takes in a keyword argument corresponding to each field.
- Props declared with `default` are initialized to their default values if an
  initializer is not provided.
- Props whose type is `T.nilable(...)` are initialized to `nil` if an
  initializer is not provided.
- Props declared as `const` may not be mutated, so they only have getters and
  don't have setters.

## Converting structs to other types

A particularly common case is to convert an struct to a Hash. Because this is so
common, this conversion has been built in (it still must be explicitly called):

```ruby
my_struct.serialize # => { "foo": 4, "quz": 0.5 }
```

Note that `bar` is skipped because it is `nil`.

## Using T::Struct as value objects

Regular Ruby structs work as value objects. An instance of a struct is
considered to be equal to another if they have the same class and the same
attributes.

The same behavior can be achieved for `T::Struct` or any class using `T::Props`
by including the `T::Props::ValueObject` module.

```ruby
class MyValueObject < T::Struct
  include T::Props::ValueObject

  prop :a, Integer
end

first = MyValueObject.new(a: 50)
second = MyValueObject.new(a: 50)

first == second # true because classes, prop definitions and values are the same

class AnotherValueObject
  include T::Props
  include T::Props::ValueObject

  prop :a, Integer
end

first = AnotherValueObject.new(a: 50)
second = AnotherValueObject.new(a: 50)

first == second # true. Same behavior as T::Struct
```

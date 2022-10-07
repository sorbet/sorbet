---
id: tstruct
title: Typed Structs via T::Struct
sidebar_label: T::Struct
---

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
  # (2) Declare fields on the struct with the `prop` and `const` DSL, followed
  #     by the type and optional rules hash.
  prop :foo, Integer
  const :bar, T.nilable(String)
  const :quz, Float, default: 0.5
end
```

## Creating and using an instance of a struct

After defining a struct class, create instances of the class with `new` as usual
with keyword arguments corresponding to each field. Then use the generated
getters and setters to access the instance's fields.

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

Props declared as `const` may not be mutated, so they only have getters and
don't have setters. You can duplicate an existing struct with new values for
props:

```ruby
new_struct = my_struct.with(bar: "baz", quz: 1.0)
new_struct.foo # => 4
new_struct.bar # => "baz"
new_struct.quz # => 1.0
```

### factory rule

Props declared with `default` are initialized to their default values if an
initializer is not provided. To evaluate a `Proc` and use the result as a
default value, declare with `factory` instead:

```ruby
class FactoryStruct < T::Struct
  const :foo, Integer, factory: -> { Random.rand(100) }
end

FactoryStruct.new.foo # => 47
FactoryStruct.new.foo # => 68
```

### raise_on_nil_write rule

Props whose type is `T.nilable(...)` are initialized to `nil` if an initializer
is not provided. To only allow nilable props to be deserialized with `nil`
values, but not set to `nil` otherwise:

```ruby
class NilWriteStruct < T::Struct
  prop :foo, T.nilable(String), raise_on_nil_write: true
end

NilWriteStruct.from_hash("foo" => nil).foo # => nil
NilWriteStruct.new(foo: nil) # error
```

### enum rule

Props can be declared with a [`T::Enum`](tenum.md) subclass as type, but it is
also possible to specify which values are allowed for a prop inline:

```ruby
class EnumStruct < T::Struct
  prop :foo, String, enum: ["bar", "qux"]
end

EnumStruct.new(foo: "other") # error
```

## Converting structs to other types

A particularly common case is to convert an struct to a Hash. Because this is so
common, this conversion has been built in (it still must be explicitly called):

```ruby
my_struct.serialize # => { "foo" => 4, "quz" => 0.5 }
```

Note that `bar` is skipped because it is `nil`. Props declared with the
`dont_store` rule are also skipped:

```ruby
class AccountStruct < T::Struct
  prop :foo, Integer
  prop :admin, T::Boolean, dont_store: true
end

AccountStruct.new(foo: 3, admin: true).serialize # => { "foo" => 3 }
```

To map a prop to a differently named Hash key, declare with the `name` rule:

```ruby
class CaseStruct < T::Struct
  prop :snake_case, Integer, name: "camelCase"
end

CaseStruct.new(snake_case: 3).serialize # => { "camelCase" => 3 }
```

## Converting from other types to structs

Similarly it is common to create a new struct from a Hash. This is also built
into `T::Struct`:

```ruby
my_hash = { "foo" => 3, "some_key" => "some_value", "admin" => true }
deserialized_struct = MyStruct.from_hash(my_hash)
deserialized_struct.foo # => 3
```

Note that Hash keys that do not match a declared prop are ignored. Props with
`dont_store` are ignored as well:

```ruby
account_struct = AccountStruct.from_hash(my_hash)
account_struct.admin # => nil
```

It is also possible to have any unknown key prevent creating a struct:

```ruby
AccountStruct.from_hash(my_hash, true) # error
AccountStruct.from_hash!(my_hash) # error
```

Props declared with the `name` rule map the Hash key to the prop:

```ruby
case_struct = CaseStruct.from_hash({ "camelCase" => 3 })
case_struct.snake_case # => 3
```

## Structs and inheritance

Sorbet does not allow inheriting from a class which inherits from `T::Struct`.

```ruby
class S < T::Struct
  prop :foo, Integer
end

class Bad < S; end # error
```

Sorbet imposes this limitation somewhat artificially, for performance. Sorbet
generates a signature for the `initialize` method of a `T::Struct` subclass. In
order to do so, it needs to know all `prop`'s defined on the class. For
performance in large codebases, Sorbet requires that it is possible to know
which methods are defined on a class purely based on syntax—Sorbet does not
allow discovering the existence of methods based on things like what a class's
superclass is, or what methods are defined on the superclass.

One common situation where inheritance may be desired is when a parent struct
declares some common props, and children structs declare their own props.

```ruby
class Parent < T::Struct
  prop :foo, Integer
end

class ChildOne < Parent # error
  prop :bar, String
end

class ChildTwo < Parent # error
  prop :quz, Symbol
end
```

We can restructure the code to use composition instead of inheritance.

```ruby
class Common < T::Struct
  prop :foo, Integer
end

class ChildOne < T::Struct
  prop :common, Common
  prop :bar, String
end

class ChildTwo < T::Struct
  prop :common, Common
  prop :quz, Symbol
end
```

If the code absolutely must use inheritance and cannot use composition, either:

- Avoid using `T::Struct`, and instead define a normal class, with things like
  `attr_reader` and an explicit `initialize` method.

- Change the superclass from `T::Struct` to `T::InexactStruct`. Sorbet will no
  longer check the types of any arguments passed to the `initialize` method on
  the subclass. This should only be used as a last resort.

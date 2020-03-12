---
id: tstruct
title: Typed Structs via T::Struct
sidebar_label: T::Struct
---

> TODO: This page is still a fragment. Contributions welcome!

While Sorbet supports using Hashes and Structs, it isn't always possible to
model them statically.

The `sorbet-runtime` gem ships with an alternative to using Ruby's `Hash` or
`Struct` classes that is more well supported: `T::Struct`. It works like this:

```ruby
# typed: true
require 'sorbet-runtime'

class MyStruct < T::Struct
  prop :x, Integer
  const :y, T.nilable(String)
  const :z, Float, default: 0.5
end
```

This is basically the same as having written code like this:

```ruby
class MyStruct < T::Struct
  sig {params(x: Integer, y: String, z: Float).void}
  def initialize(x:, y: nil, z: 0.5); ...; end

  sig {returns(Integer)}
  def x; ...; end

  sig {params(arg0: Integer).returns(Integer)}
  def x=(arg0); ...; end

  sig {returns(T.nilable(String))}
  def y; ...; end

  sig {returns(Float)}
  def z; ...; end
end
```

It can be used like this:

```ruby
my_struct = MyStruct.new(x: 3)
my_struct.x # => 3
my_struct.y # => nil
my_struct.z # => 0.5
my_struct.x = 4
my_struct.x # => 4
```

## Converting structs to other types

A particularly common case is to convert an struct to a Hash. Because this is so
common, this conversion has been built in (it still must be explicitly called):

```ruby
my_struct.serialize # => { "x": 3 }
```

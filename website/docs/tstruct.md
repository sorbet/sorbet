---
id: tstruct
title: Typed Structs
---

> TODO: This page is still a fragment. Contributions welcome!

While Sorbet supports using `Hash`es and `Struct`s, using they aren't always
possible to model well statically.

The `sorbet-runtime` gem ships with an alternative to using Ruby's `Hash` or
`Struct` classes that is more well supported: `T::Struct`. It works like this:

```ruby
# typed: true
require 'sorbet-runtime'

class MyStruct < T::Struct
  prop :x, Integer
  const :y, T.nilable(String)
end
```

This is basically the same as having written code like this:

```ruby
class MyStruct < T::Struct
  def initialize(x:, y: nil)
    @x = x
    @y = y
  end

  sig {returns(Integer)}
  def x; @x; end

  sig {params(x: Integer).returns(Integer)}
  def x=(x); @x = x; end

  sig {returns(T.nilable(String))}
  def y; @y; end
end
```

It can be used like this:

```ruby
my_struct = MyStruct.new(x: 0)
puts my_struct.x # => 0
puts my_struct.y # => nil
```

> **Note**: `T::Struct` does not currently statically type check the call to the
> constructor. This is planned but not yet implemented.

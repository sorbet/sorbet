# typed: __STDLIB_INTERNAL

# A class which allows both internal and external iteration.
#
# An [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) can be
# created by the following methods.
# *   Kernel#to\_enum
# *   Kernel#enum\_for
# *   [`Enumerator.new`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html#method-c-new)
#
#
# Most methods have two forms: a block form where the contents are evaluated for
# each item in the enumeration, and a non-block form which returns a new
# [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) wrapping
# the iteration.
#
# ```ruby
# enumerator = %w(one two three).each
# puts enumerator.class # => Enumerator
#
# enumerator.each_with_object("foo") do |item, obj|
#   puts "#{obj}: #{item}"
# end
#
# # foo: one
# # foo: two
# # foo: three
#
# enum_with_obj = enumerator.each_with_object("foo")
# puts enum_with_obj.class # => Enumerator
#
# enum_with_obj.each do |item, obj|
#   puts "#{obj}: #{item}"
# end
#
# # foo: one
# # foo: two
# # foo: three
# ```
#
# This allows you to chain Enumerators together. For example, you can map a
# list's elements to strings containing the index and the element as a string
# via:
#
# ```ruby
# puts %w[foo bar baz].map.with_index { |w, i| "#{i}:#{w}" }
# # => ["0:foo", "1:bar", "2:baz"]
# ```
#
# An [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) can
# also be used as an external iterator. For example,
# [`Enumerator#next`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html#method-i-next)
# returns the next value of the iterator or raises
# [`StopIteration`](https://docs.ruby-lang.org/en/2.6.0/StopIteration.html) if
# the [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is at
# the end.
#
# ```ruby
# e = [1,2,3].each   # returns an enumerator object.
# puts e.next   # => 1
# puts e.next   # => 2
# puts e.next   # => 3
# puts e.next   # raises StopIteration
# ```
#
# You can use this to implement an internal iterator as follows:
#
# ```ruby
# def ext_each(e)
#   while true
#     begin
#       vs = e.next_values
#     rescue StopIteration
#       return $!.result
#     end
#     y = yield(*vs)
#     e.feed y
#   end
# end
#
# o = Object.new
#
# def o.each
#   puts yield
#   puts yield(1)
#   puts yield(1, 2)
#   3
# end
#
# # use o.each as an internal iterator directly.
# puts o.each {|*x| puts x; [:b, *x] }
# # => [], [:b], [1], [:b, 1], [1, 2], [:b, 1, 2], 3
#
# # convert o.each to an external iterator for
# # implementing an internal iterator.
# puts ext_each(o.to_enum) {|*x| puts x; [:b, *x] }
# # => [], [:b], [1], [:b, 1], [1, 2], [:b, 1, 2], 3
# ```
class Enumerator < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  # Iterates over the block according to how this
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) was
  # constructed. If no block and no arguments are given, returns self.
  #
  # ### Examples
  #
  # ```ruby
  # "Hello, world!".scan(/\w+/)                     #=> ["Hello", "world"]
  # "Hello, world!".to_enum(:scan, /\w+/).to_a      #=> ["Hello", "world"]
  # "Hello, world!".to_enum(:scan).each(/\w+/).to_a #=> ["Hello", "world"]
  #
  # obj = Object.new
  #
  # def obj.each_arg(a, b=:b, *rest)
  #   yield a
  #   yield b
  #   yield rest
  #   :method_returned
  # end
  #
  # enum = obj.to_enum :each_arg, :a, :x
  #
  # enum.each.to_a                  #=> [:a, :x, []]
  # enum.each.equal?(enum)          #=> true
  # enum.each { |elm| elm }         #=> :method_returned
  #
  # enum.each(:y, :z).to_a          #=> [:a, :x, [:y, :z]]
  # enum.each(:y, :z).equal?(enum)  #=> false
  # enum.each(:y, :z) { |elm| elm } #=> :method_returned
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end

  # Sets the value to be returned by the next yield inside `e`.
  #
  # If the value is not set, the yield returns nil.
  #
  # This value is cleared after being yielded.
  #
  # ```ruby
  # # Array#map passes the array's elements to "yield" and collects the
  # # results of "yield" as an array.
  # # Following example shows that "next" returns the passed elements and
  # # values passed to "feed" are collected as an array which can be
  # # obtained by StopIteration#result.
  # e = [1,2,3].map
  # p e.next           #=> 1
  # e.feed "a"
  # p e.next           #=> 2
  # e.feed "b"
  # p e.next           #=> 3
  # e.feed "c"
  # begin
  #   e.next
  # rescue StopIteration
  #   p $!.result      #=> ["a", "b", "c"]
  # end
  #
  # o = Object.new
  # def o.each
  #   x = yield         # (2) blocks
  #   p x               # (5) => "foo"
  #   x = yield         # (6) blocks
  #   p x               # (8) => nil
  #   x = yield         # (9) blocks
  #   p x               # not reached w/o another e.next
  # end
  #
  # e = o.to_enum
  # e.next              # (1)
  # e.feed "foo"        # (3)
  # e.next              # (4)
  # e.next              # (7)
  #                     # (10)
  # ```
  sig do
    params(
        arg0: Elem,
    )
    .returns(NilClass)
  end
  def feed(arg0); end

  sig do
    params(
        arg0: T.any(Integer, T.proc.returns(Integer)),
        blk: T.proc.params(arg0: Enumerator::Yielder).void,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil), &blk); end

  # Creates a printable version of *e*.
  sig {returns(String)}
  def inspect(); end

  # Returns the next object in the enumerator, and move the internal position
  # forward. When the position reached at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.6.0/StopIteration.html) is
  # raised.
  #
  # ### Example
  #
  # ```ruby
  # a = [1,2,3]
  # e = a.to_enum
  # p e.next   #=> 1
  # p e.next   #=> 2
  # p e.next   #=> 3
  # p e.next   #raises StopIteration
  # ```
  #
  # Note that enumeration sequence by `next` does not affect other non-external
  # enumeration methods, unless the underlying iteration methods itself has
  # side-effect, e.g.
  # [`IO#each_line`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-each_line).
  sig {returns(Elem)}
  def next(); end

  # Returns the next object as an array in the enumerator, and move the internal
  # position forward. When the position reached at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.6.0/StopIteration.html) is
  # raised.
  #
  # This method can be used to distinguish `yield` and `yield nil`.
  #
  # ### Example
  #
  # ```ruby
  # o = Object.new
  # def o.each
  #   yield
  #   yield 1
  #   yield 1, 2
  #   yield nil
  #   yield [1, 2]
  # end
  # e = o.to_enum
  # p e.next_values
  # p e.next_values
  # p e.next_values
  # p e.next_values
  # p e.next_values
  # e = o.to_enum
  # p e.next
  # p e.next
  # p e.next
  # p e.next
  # p e.next
  #
  # ## yield args       next_values      next
  # #  yield            []               nil
  # #  yield 1          [1]              1
  # #  yield 1, 2       [1, 2]           [1, 2]
  # #  yield nil        [nil]            nil
  # #  yield [1, 2]     [[1, 2]]         [1, 2]
  # ```
  #
  # Note that `next_values` does not affect other non-external enumeration
  # methods unless underlying iteration method itself has side-effect, e.g.
  # [`IO#each_line`](https://docs.ruby-lang.org/en/2.6.0/IO.html#method-i-each_line).
  sig {returns(T::Array[Elem])}
  def next_values(); end

  # Returns the next object in the enumerator, but doesn't move the internal
  # position forward. If the position is already at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.6.0/StopIteration.html) is
  # raised.
  #
  # ### Example
  #
  # ```ruby
  # a = [1,2,3]
  # e = a.to_enum
  # p e.next   #=> 1
  # p e.peek   #=> 2
  # p e.peek   #=> 2
  # p e.peek   #=> 2
  # p e.next   #=> 2
  # p e.next   #=> 3
  # p e.peek   #raises StopIteration
  # ```
  sig {returns(Elem)}
  def peek(); end

  # Returns the next object as an array, similar to
  # [`Enumerator#next_values`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html#method-i-next_values),
  # but doesn't move the internal position forward. If the position is already
  # at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.6.0/StopIteration.html) is
  # raised.
  #
  # ### Example
  #
  # ```ruby
  # o = Object.new
  # def o.each
  #   yield
  #   yield 1
  #   yield 1, 2
  # end
  # e = o.to_enum
  # p e.peek_values    #=> []
  # e.next
  # p e.peek_values    #=> [1]
  # p e.peek_values    #=> [1]
  # e.next
  # p e.peek_values    #=> [1, 2]
  # e.next
  # p e.peek_values    # raises StopIteration
  # ```
  sig {returns(T::Array[Elem])}
  def peek_values(); end

  # Rewinds the enumeration sequence to the beginning.
  #
  # If the enclosed object responds to a "rewind" method, it is called.
  sig {returns(T.self_type)}
  def rewind(); end

  # Returns the size of the enumerator, or `nil` if it can't be calculated
  # lazily.
  #
  # ```ruby
  # (1..100).to_a.permutation(4).size # => 94109400
  # loop.size # => Float::INFINITY
  # (1..100).drop_while.size # => nil
  # ```
  sig {returns(T.nilable(T.any(Integer, Float)))}
  def size(); end

  # Iterates the given block for each element with an index, which starts from
  # `offset`. If no block is given, returns a new
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) that
  # includes the index, starting from `offset`
  #
  # `offset`
  # :   the starting index to use
  sig do
    params(
        offset: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Integer).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig do
    params(
        offset: Integer,
    )
    .returns(T::Enumerator[[Elem, Integer]])
  end
  def with_index(offset=0, &blk); end

  # Iterates the given block for each element with an arbitrary object, `obj`,
  # and returns `obj`
  #
  # If no block is given, returns a new
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html).
  #
  # ### Example
  #
  # ```ruby
  # to_three = Enumerator.new do |y|
  #   3.times do |x|
  #     y << x
  #   end
  # end
  #
  # to_three_with_string = to_three.with_object("foo")
  # to_three_with_string.each do |x,string|
  #   puts "#{string}: #{x}"
  # end
  #
  # # => foo:0
  # # => foo:1
  # # => foo:2
  # ```
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
        blk: T.proc.params(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Enumerator[[Elem, T.type_parameter(:U)]])
  end
  def with_object(arg0, &blk); end
end

# [`Generator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/Generator.html)
class Enumerator::Generator < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

# [`Lazy`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/Lazy.html)
class Enumerator::Lazy < Enumerator
  extend T::Generic
  Elem = type_member(:out)

  # Creates a new
  # [`Lazy`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/Lazy.html)
  # enumerator. When the enumerator is actually enumerated (e.g. by calling
  # [`force`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/Lazy.html#method-i-force)),
  # `obj` will be enumerated and each value passed to the given block. The block
  # can yield values back using `yielder`. For example, to create a method
  # `filter_map` in both lazy and non-lazy fashions:
  #
  # ```ruby
  # module Enumerable
  #   def filter_map(&block)
  #     map(&block).compact
  #   end
  # end
  #
  # class Enumerator::Lazy
  #   def filter_map
  #     Lazy.new(self) do |yielder, *values|
  #       result = yield *values
  #       yielder << result if result
  #     end
  #   end
  # end
  #
  # (1..Float::INFINITY).lazy.filter_map{|i| i*i if i.even?}.first(5)
  #     # => [4, 16, 36, 64, 100]
  # ```
  def self.new(*_); end

  def chunk(*_); end

  def chunk_while(*_); end

  # Returns a new array with the results of running *block* once for every
  # element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..4).map { |i| i*i }      #=> [1, 4, 9, 16]
  # (1..4).collect { "cat"  }   #=> ["cat", "cat", "cat", "cat"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(Enumerator::Lazy[T.type_parameter(:U)])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def collect(&blk); end

  # Returns a new array with the concatenated results of running *block* once
  # for every element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # [1, 2, 3, 4].flat_map { |e| [e, -e] } #=> [1, -1, 2, -2, 3, -3, 4, -4]
  # [[1, 2], [3, 4]].flat_map { |e| e + [100] } #=> [1, 2, 100, 3, 4, 100]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T::Enumerator[T.type_parameter(:U)]),
    )
    .returns(Enumerator::Lazy[T.type_parameter(:U)])
  end
  def collect_concat(&blk); end

  # Drops first n elements from *enum*, and returns rest elements in an array.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.drop(3)             #=> [4, 5, 0]
  # ```
  sig do
    params(
        n: Integer,
    )
    .returns(Enumerator::Lazy[Elem])
  end
  def drop(n); end

  # Drops elements up to, but not including, the first element for which the
  # block returns `nil` or `false` and returns an array containing the remaining
  # elements.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.drop_while { |i| i < 3 }   #=> [3, 4, 5, 0]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Enumerator::Lazy[Elem])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def drop_while(&blk); end

  # Similar to Kernel#to\_enum, except it returns a lazy enumerator. This makes
  # it easy to define
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) methods
  # that will naturally remain lazy if called from a lazy enumerator.
  #
  # For example, continuing from the example in Kernel#to\_enum:
  #
  # ```ruby
  # # See Kernel#to_enum for the definition of repeat
  # r = 1..Float::INFINITY
  # r.repeat(2).first(5) # => [1, 1, 2, 2, 3]
  # r.repeat(2).class # => Enumerator
  # r.repeat(2).map{|n| n ** 2}.first(5) # => endless loop!
  # # works naturally on lazy enumerator:
  # r.lazy.repeat(2).class # => Enumerator::Lazy
  # r.lazy.repeat(2).map{|n| n ** 2}.first(5) # => [1, 1, 4, 4, 9]
  # ```
  def enum_for(*_); end

  # Alias for:
  # [`to_a`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/Lazy.html#method-i-to_a)
  def force(*_); end

  # Returns an array containing all elements of `enum` for which the given
  # `block` returns a true value.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).find_all { |i|  i % 3 == 0 }   #=> [3, 6, 9]
  #
  # [1,2,3,4,5].select { |num|  num.even?  }   #=> [2, 4]
  #
  # [:foo, :bar].filter { |x| x == :foo }   #=> [:foo]
  # ```
  #
  # See also
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-reject).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Enumerator::Lazy[Elem])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def find_all(&blk); end

  ### This signature is wrong; See entire note on Enumerator#flat_map

  # Returns a new array with the concatenated results of running *block* once
  # for every element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # [1, 2, 3, 4].flat_map { |e| [e, -e] } #=> [1, -1, 2, -2, 3, -3, 4, -4]
  # [[1, 2], [3, 4]].flat_map { |e| e + [100] } #=> [1, 2, 100, 3, 4, 100]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T::Enumerable[T.type_parameter(:U)]),
    )
    .returns(Enumerator::Lazy[T.type_parameter(:U)])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def flat_map(&blk); end

  # Returns an array of every element in *enum* for which `Pattern === element`.
  # If the optional *block* is supplied, each matching element is passed to it,
  # and the block's result is stored in the output array.
  #
  # ```ruby
  # (1..100).grep 38..44   #=> [38, 39, 40, 41, 42, 43, 44]
  # c = IO.constants
  # c.grep(/SEEK/)         #=> [:SEEK_SET, :SEEK_CUR, :SEEK_END]
  # res = c.grep(/SEEK/) { |v| IO.const_get(v) }
  # res                    #=> [0, 1, 2]
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(Enumerator::Lazy[Elem])
  end
  sig do
    type_parameters(:U).params(
        arg0: BasicObject,
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(Enumerator::Lazy[T.type_parameter(:U)])
  end
  def grep(arg0, &blk); end

  def grep_v(_); end

  def lazy; end

  # Returns a new array with the results of running *block* once for every
  # element in *enum*.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (1..4).map { |i| i*i }      #=> [1, 4, 9, 16]
  # (1..4).collect { "cat"  }   #=> ["cat", "cat", "cat", "cat"]
  # ```
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(Enumerator::Lazy[T.type_parameter(:U)])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def map(&blk); end

  # Returns an array for all elements of `enum` for which the given `block`
  # returns `false`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).reject { |i|  i % 3 == 0 }   #=> [1, 2, 4, 5, 7, 8, 10]
  #
  # [1, 2, 3, 4, 5].reject { |num| num.even? } #=> [1, 3, 5]
  # ```
  #
  # See also
  # [`Enumerable#find_all`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-find_all).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Enumerator::Lazy[Elem])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def reject(&blk); end

  # Returns an array containing all elements of `enum` for which the given
  # `block` returns a true value.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.6.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).find_all { |i|  i % 3 == 0 }   #=> [3, 6, 9]
  #
  # [1,2,3,4,5].select { |num|  num.even?  }   #=> [2, 4]
  #
  # [:foo, :bar].filter { |x| x == :foo }   #=> [:foo]
  # ```
  #
  # See also
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-reject).
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Enumerator::Lazy[Elem])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def select(&blk); end

  def slice_after(*_); end

  def slice_before(*_); end

  def slice_when(*_); end

  # Returns first n elements from *enum*.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.take(3)             #=> [1, 2, 3]
  # a.take(30)            #=> [1, 2, 3, 4, 5, 0]
  # ```
  sig do
    params(
        n: Integer,
    )
    .returns(Enumerator::Lazy[Elem])
  end
  def take(n); end

  # Passes elements to the block until the block returns `nil` or `false`, then
  # stops iterating and returns an array of all prior elements.
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # a = [1, 2, 3, 4, 5, 0]
  # a.take_while { |i| i < 3 }   #=> [1, 2]
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Enumerator::Lazy[Elem])
  end
  sig {returns(Enumerator::Lazy[Elem])}
  def take_while(&blk); end

  # Similar to Kernel#to\_enum, except it returns a lazy enumerator. This makes
  # it easy to define
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html) methods
  # that will naturally remain lazy if called from a lazy enumerator.
  #
  # For example, continuing from the example in Kernel#to\_enum:
  #
  # ```ruby
  # # See Kernel#to_enum for the definition of repeat
  # r = 1..Float::INFINITY
  # r.repeat(2).first(5) # => [1, 1, 2, 2, 3]
  # r.repeat(2).class # => Enumerator
  # r.repeat(2).map{|n| n ** 2}.first(5) # => endless loop!
  # # works naturally on lazy enumerator:
  # r.lazy.repeat(2).class # => Enumerator::Lazy
  # r.lazy.repeat(2).map{|n| n ** 2}.first(5) # => [1, 1, 4, 4, 9]
  # ```
  def to_enum(*_); end

  def uniq; end

  def zip(*_); end
end

# [`Yielder`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/Yielder.html)
class Enumerator::Yielder < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .void
  end
  def <<(*arg0); end

  sig do
    params(
        arg0: BasicObject,
    )
    .void
  end
  def yield(*arg0); end
end

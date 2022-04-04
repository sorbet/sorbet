# typed: __STDLIB_INTERNAL

# A class which allows both internal and external iteration.
#
# An [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) can be
# created by the following methods.
# *   [`Object#to_enum`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_enum)
# *   [`Object#enum_for`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-enum_for)
# *   [`Enumerator.new`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html#method-c-new)
#
#
# Most methods have two forms: a block form where the contents are evaluated for
# each item in the enumeration, and a non-block form which returns a new
# [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) wrapping
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
# An [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) can
# also be used as an external iterator. For example,
# [`Enumerator#next`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html#method-i-next)
# returns the next value of the iterator or raises
# [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html) if
# the [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is at
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
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) was
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
      blk: T.proc.params(arg0: Elem).returns(BasicObject)
    )
      .returns(T.untyped)
  end
  sig { returns(T.self_type) }
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
      arg0: Elem
    )
      .returns(NilClass)
  end
  def feed(arg0); end

  sig do
    params(
      arg0: T.any(Integer, T.proc.returns(Integer)),
      blk: T.proc.params(arg0: Enumerator::Yielder).void
    )
      .void
  end
  def initialize(arg0 = T.unsafe(nil), &blk); end

  # Creates a printable version of *e*.
  sig { returns(String) }
  def inspect(); end

  # Returns the next object in the enumerator, and move the internal position
  # forward. When the position reached at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html) is
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
  # [`IO#each_line`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_line).
  sig { returns(Elem) }
  def next(); end

  # Returns the next object as an array in the enumerator, and move the internal
  # position forward. When the position reached at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html) is
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
  # [`IO#each_line`](https://docs.ruby-lang.org/en/2.7.0/IO.html#method-i-each_line).
  sig { returns(T::Array[Elem]) }
  def next_values(); end

  # Returns the next object in the enumerator, but doesn't move the internal
  # position forward. If the position is already at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html) is
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
  sig { returns(Elem) }
  def peek(); end

  # Returns the next object as an array, similar to
  # [`Enumerator#next_values`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html#method-i-next_values),
  # but doesn't move the internal position forward. If the position is already
  # at the end,
  # [`StopIteration`](https://docs.ruby-lang.org/en/2.7.0/StopIteration.html) is
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
  sig { returns(T::Array[Elem]) }
  def peek_values(); end

  # Rewinds the enumeration sequence to the beginning.
  #
  # If the enclosed object responds to a "rewind" method, it is called.
  sig { returns(T.self_type) }
  def rewind(); end

  # Returns the size of the enumerator, or `nil` if it can't be calculated
  # lazily.
  #
  # ```ruby
  # (1..100).to_a.permutation(4).size # => 94109400
  # loop.size # => Float::INFINITY
  # (1..100).drop_while.size # => nil
  # ```
  sig { returns(T.nilable(T.any(Integer, Float))) }
  def size(); end

  # Iterates the given block for each element with an index, which starts from
  # `offset`. If no block is given, returns a new
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) that
  # includes the index, starting from `offset`
  #
  # `offset`
  # :   the starting index to use
  sig do
    params(
      offset: Integer,
      blk: T.proc.params(arg0: Elem, arg1: Integer).returns(BasicObject)
    )
      .returns(T.untyped)
  end
  sig do
    params(
      offset: Integer
    )
      .returns(T::Enumerator[[Elem, Integer]])
  end
  def with_index(offset = 0, &blk); end

  # Iterates the given block for each element with an arbitrary object, `obj`,
  # and returns `obj`
  #
  # If no block is given, returns a new
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html).
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
      blk: T.proc.params(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject)
    )
                       .returns(T.untyped)
  end
  sig do
    type_parameters(:U).params(
      arg0: T.type_parameter(:U)
    )
                       .returns(T::Enumerator[[Elem, T.type_parameter(:U)]])
  end
  def with_object(arg0, &blk); end
end

# [`Generator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Generator.html)
class Enumerator::Generator < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

# [`Enumerator::Lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html)
# is a special type of
# [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html), that
# allows constructing chains of operations without evaluating them immediately,
# and evaluating values on as-needed basis. In order to do so it redefines most
# of [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) methods
# so that they just construct another lazy enumerator.
#
# [`Enumerator::Lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html)
# can be constructed from any
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) with the
# [`Enumerable#lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-lazy)
# method.
#
# ```ruby
# lazy = (1..Float::INFINITY).lazy.select(&:odd?).drop(10).take_while { |i| i < 30 }
# # => #<Enumerator::Lazy: #<Enumerator::Lazy: #<Enumerator::Lazy: #<Enumerator::Lazy: 1..Infinity>:select>:drop(10)>:take_while>
# ```
#
# The real enumeration is performed when any non-redefined
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) method is
# called, like
# [`Enumerable#first`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-first)
# or
# [`Enumerable#to_a`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-to_a)
# (the latter is aliased as
# [`force`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-force)
# for more semantic code):
#
# ```ruby
# lazy.first(2)
# #=> [21, 23]
#
# lazy.force
# #=> [21, 23, 25, 27, 29]
# ```
#
# Note that most
# [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) methods
# that could be called with or without a block, on
# [`Enumerator::Lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html)
# will always require a block:
#
# ```ruby
# [1, 2, 3].map       #=> #<Enumerator: [1, 2, 3]:map>
# [1, 2, 3].lazy.map  # ArgumentError: tried to call lazy map without a block
# ```
#
# This class allows idiomatic calculations on long or infinite sequences, as
# well as chaining of calculations without constructing intermediate arrays.
#
# Example for working with a slowly calculated sequence:
#
# ```ruby
# require 'open-uri'
#
# # This will fetch all URLs before selecting
# # necessary data
# URLS.map { |u| JSON.parse(open(u).read) }
#   .select { |data| data.key?('stats') }
#   .first(5)
#
# # This will fetch URLs one-by-one, only till
# # there is enough data to satisfy the condition
# URLS.lazy.map { |u| JSON.parse(open(u).read) }
#   .select { |data| data.key?('stats') }
#   .first(5)
# ```
#
# Ending a chain with ".eager" generates a non-lazy enumerator, which is
# suitable for returning or passing to another method that expects a normal
# enumerator.
#
# ```ruby
# def active_items
#   groups
#     .lazy
#     .flat_map(&:items)
#     .reject(&:disabled)
#     .eager
# end
#
# # This works lazily; if a checked item is found, it stops
# # iteration and does not look into remaining groups.
# first_checked = active_items.find(&:checked)
#
# # This returns an array of items like a normal enumerator does.
# all_checked = active_items.select(&:checked)
# ```
class Enumerator::Lazy < Enumerator
  extend T::Generic
  Elem = type_member(:out)

  # Creates a new
  # [`Lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html)
  # enumerator. When the enumerator is actually enumerated (e.g. by calling
  # [`force`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-force)),
  # `obj` will be enumerated and each value passed to the given block. The block
  # can yield values back using `yielder`. For example, to create a "filter+map"
  # enumerator:
  #
  # ```ruby
  # def filter_map(sequence)
  #   Lazy.new(sequence) do |yielder, *values|
  #     result = yield *values
  #     yielder << result if result
  #   end
  # end
  #
  # filter_map(1..Float::INFINITY) {|i| i*i if i.even?}.first(5)
  # #=> [4, 16, 36, 64, 100]
  # ```
  def self.new(*_); end

  # Like
  # [`Enumerable#chunk`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk),
  # but chains operation to be lazy-evaluated.
  def chunk(*_); end

  # Like
  # [`Enumerable#chunk_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-chunk_while),
  # but chains operation to be lazy-evaluated.
  def chunk_while(*_); end

  # Like
  # [`Enumerable#map`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-map),
  # but chains operation to be lazy-evaluated.
  #
  # ```ruby
  # (1..Float::INFINITY).lazy.map {|i| i**2 }
  # #=> #<Enumerator::Lazy: #<Enumerator::Lazy: 1..Infinity>:map>
  # (1..Float::INFINITY).lazy.map {|i| i**2 }.first(3)
  # #=> [1, 4, 9]
  # ```
  #
  #
  # Also aliased as:
  # [`_enumerable_collect`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_collect)
  sig do
    type_parameters(:U).params(
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U))
    )
                       .returns(T::Enumerator::Lazy[T.type_parameter(:U)])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def collect(&blk); end

  # Returns a new lazy enumerator with the concatenated results of running
  # `block` once for every element in the lazy enumerator.
  #
  # ```ruby
  # ["foo", "bar"].lazy.flat_map {|i| i.each_char.lazy}.force
  # #=> ["f", "o", "o", "b", "a", "r"]
  # ```
  #
  # A value `x` returned by `block` is decomposed if either of the following
  # conditions is true:
  #
  # *   `x` responds to both each and force, which means that `x` is a lazy
  #     enumerator.
  # *   `x` is an array or responds to to\_ary.
  #
  #
  # Otherwise, `x` is contained as-is in the return value.
  #
  # ```ruby
  # [{a:1}, {b:2}].lazy.flat_map {|i| i}.force
  # #=> [{:a=>1}, {:b=>2}]
  # ```
  #
  #
  # Also aliased as:
  # [`_enumerable_collect_concat`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_collect_concat)
  sig do
    type_parameters(:U).params(
      blk: T.proc.params(arg0: Elem).returns(T::Enumerator[T.type_parameter(:U)])
    )
                       .returns(T::Enumerator::Lazy[T.type_parameter(:U)])
  end
  def collect_concat(&blk); end

  # Like
  # [`Enumerable#drop`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-drop),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_drop`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_drop)
  sig do
    params(
      n: Integer
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  def drop(n); end

  # Like
  # [`Enumerable#drop_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-drop_while),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_drop_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_drop_while)
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject)
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def drop_while(&blk); end

  # Similar to
  # [`Object#to_enum`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_enum),
  # except it returns a lazy enumerator. This makes it easy to define
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) methods
  # that will naturally remain lazy if called from a lazy enumerator.
  #
  # For example, continuing from the example in
  # [`Object#to_enum`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_enum):
  #
  # ```ruby
  # # See Object#to_enum for the definition of repeat
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
  # [`to_a`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-to_a)
  def force(*_); end

  # Like
  # [`Enumerable#select`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-select),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_find_all`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_find_all)
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject)
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def find_all(&blk); end

  ### This signature is wrong; See entire note on Enumerator#flat_map

  # Returns a new lazy enumerator with the concatenated results of running
  # `block` once for every element in the lazy enumerator.
  #
  # ```ruby
  # ["foo", "bar"].lazy.flat_map {|i| i.each_char.lazy}.force
  # #=> ["f", "o", "o", "b", "a", "r"]
  # ```
  #
  # A value `x` returned by `block` is decomposed if either of the following
  # conditions is true:
  #
  # *   `x` responds to both each and force, which means that `x` is a lazy
  #     enumerator.
  # *   `x` is an array or responds to to\_ary.
  #
  #
  # Otherwise, `x` is contained as-is in the return value.
  #
  # ```ruby
  # [{a:1}, {b:2}].lazy.flat_map {|i| i}.force
  # #=> [{:a=>1}, {:b=>2}]
  # ```
  #
  #
  # Also aliased as:
  # [`_enumerable_flat_map`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_flat_map)
  sig do
    type_parameters(:U).params(
      blk: T.proc.params(arg0: Elem).returns(T::Enumerable[T.type_parameter(:U)])
    )
                       .returns(T::Enumerator::Lazy[T.type_parameter(:U)])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def flat_map(&blk); end

  # Like
  # [`Enumerable#grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_grep`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_grep)
  sig do
    params(
      arg0: BasicObject
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  sig do
    type_parameters(:U).params(
      arg0: BasicObject,
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U))
    )
                       .returns(T::Enumerator::Lazy[T.type_parameter(:U)])
  end
  def grep(arg0, &blk); end

  # Like
  # [`Enumerable#grep_v`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-grep_v),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_grep_v`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_grep_v)
  def grep_v(_); end

  # Returns self.
  sig {returns(T.self_type)}
  def lazy; end

  # Like
  # [`Enumerable#map`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-map),
  # but chains operation to be lazy-evaluated.
  #
  # ```ruby
  # (1..Float::INFINITY).lazy.map {|i| i**2 }
  # #=> #<Enumerator::Lazy: #<Enumerator::Lazy: 1..Infinity>:map>
  # (1..Float::INFINITY).lazy.map {|i| i**2 }.first(3)
  # #=> [1, 4, 9]
  # ```
  #
  #
  # Also aliased as:
  # [`_enumerable_map`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_map)
  sig do
    type_parameters(:U).params(
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U))
    )
                       .returns(T::Enumerator::Lazy[T.type_parameter(:U)])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def map(&blk); end

  # Like
  # [`Enumerable#reject`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-reject),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_reject`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_reject)
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject)
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def reject(&blk); end

  # Like
  # [`Enumerable#select`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-select),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_select`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_select)
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject)
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def select(&blk); end

  # Returns a new lazy enumerator containing the truthy results (everything except `false`
  # or `nil`) of running the `block` for every element in `enum`.
  #
  # If no block is given, a
  # [`Lazy`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html) is
  # returned instead.
  #
  # ```ruby
  # (1..10).lazy.filter_map { |i| i * 2 if i.even? } #=> #<Enumerator::Lazy: #<Enumerator::Lazy: 1..10>:filter_map>
  # ```
  sig do
    type_parameters(:T)
      .params(blk: T.proc.params(arg0: Elem).returns(T.any(NilClass, FalseClass, T.type_parameter(:T))))
      .returns(T::Enumerator::Lazy[T.type_parameter(:T)])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def filter_map(&blk); end

  # Like
  # [`Enumerable#slice_after`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_after),
  # but chains operation to be lazy-evaluated.
  def slice_after(*_); end

  # Like
  # [`Enumerable#slice_before`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_before),
  # but chains operation to be lazy-evaluated.
  def slice_before(*_); end

  # Like
  # [`Enumerable#slice_when`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-slice_when),
  # but chains operation to be lazy-evaluated.
  def slice_when(*_); end

  # Like
  # [`Enumerable#take`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-take),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_take`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_take)
  sig do
    params(
      n: Integer
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  def take(n); end

  # Like
  # [`Enumerable#take_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-take_while),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_take_while`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_take_while)
  sig do
    params(
      blk: T.proc.params(arg0: Elem).returns(BasicObject)
    )
      .returns(T::Enumerator::Lazy[Elem])
  end
  sig { returns(T::Enumerator::Lazy[Elem]) }
  def take_while(&blk); end

  # Similar to
  # [`Object#to_enum`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_enum),
  # except it returns a lazy enumerator. This makes it easy to define
  # [`Enumerable`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html) methods
  # that will naturally remain lazy if called from a lazy enumerator.
  #
  # For example, continuing from the example in
  # [`Object#to_enum`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_enum):
  #
  # ```ruby
  # # See Object#to_enum for the definition of repeat
  # r = 1..Float::INFINITY
  # r.repeat(2).first(5) # => [1, 1, 2, 2, 3]
  # r.repeat(2).class # => Enumerator
  # r.repeat(2).map{|n| n ** 2}.first(5) # => endless loop!
  # # works naturally on lazy enumerator:
  # r.lazy.repeat(2).class # => Enumerator::Lazy
  # r.lazy.repeat(2).map{|n| n ** 2}.first(5) # => [1, 1, 4, 4, 9]
  # ```
  def to_enum(*_); end

  # Like
  # [`Enumerable#uniq`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-uniq),
  # but chains operation to be lazy-evaluated.
  #
  # Also aliased as:
  # [`_enumerable_uniq`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_uniq)
  def uniq; end

  # Like
  # [`Enumerable#zip`](https://docs.ruby-lang.org/en/2.7.0/Enumerable.html#method-i-zip),
  # but chains operation to be lazy-evaluated. However, if a block is given to
  # zip, values are enumerated immediately.
  #
  # Also aliased as:
  # [`_enumerable_zip`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Lazy.html#method-i-_enumerable_zip)
  def zip(*_); end
end

# [`Yielder`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/Yielder.html)
class Enumerator::Yielder < Object
  sig do
    params(
      arg0: BasicObject
    )
      .void
  end
  def <<(*arg0); end

  sig do
    params(
      arg0: BasicObject
    )
      .void
  end
  def yield(*arg0); end
end

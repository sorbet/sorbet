# typed: __STDLIB_INTERNAL

# A `Range` represents an interval---a set of values with a beginning and an
# end. Ranges may be constructed using the *s*`..`*e* and *s*`...`*e* literals,
# or with
# [`Range::new`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-c-new).
# Ranges constructed using `..` run from the beginning to the end inclusively.
# Those created using `...` exclude the end value. When used as an iterator,
# ranges return each value in the sequence.
#
# ```ruby
# (-1..-5).to_a      #=> []
# (-5..-1).to_a      #=> [-5, -4, -3, -2, -1]
# ('a'..'e').to_a    #=> ["a", "b", "c", "d", "e"]
# ('a'...'e').to_a   #=> ["a", "b", "c", "d"]
# ```
#
# ## Endless Ranges
#
# An "endless range" represents a semi-infinite range. Literal notation for an
# endless range is:
#
# ```ruby
# (1..)
# # or similarly
# (1...)
# ```
#
# Which is equivalent to
#
# ```ruby
# (1..nil)  # or similarly (1...nil)
# Range.new(1, nil) # or Range.new(1, nil, true)
# ```
#
# Endless ranges are useful, for example, for idiomatic slicing of arrays:
#
# ```ruby
# [1, 2, 3, 4, 5][2...]   # => [3, 4, 5]
# ```
#
# Some implementation details:
#
# *   `end` of endless range is `nil`;
# *   `each` of endless range enumerates infinite sequence (may be useful in
#     combination with
#     [`Enumerable#take_while`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html#method-i-take_while)
#     or similar methods);
# *   `(1..)` and `(1...)` are not equal, although technically representing the
#     same sequence.
#
#
# ## Custom Objects in Ranges
#
# Ranges can be constructed using any objects that can be compared using the
# `<=>` operator. Methods that treat the range as a sequence (#each and methods
# inherited from
# [`Enumerable`](https://docs.ruby-lang.org/en/2.6.0/Enumerable.html)) expect
# the begin object to implement a `succ` method to return the next object in
# sequence. The
# [`step`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-step) and
# [`include?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-include-3F)
# methods require the begin object to implement `succ` or to be numeric.
#
# In the `Xs` class below both `<=>` and `succ` are implemented so `Xs` can be
# used to construct ranges. Note that the
# [`Comparable`](https://docs.ruby-lang.org/en/2.6.0/Comparable.html) module is
# included so the `==` method is defined in terms of `<=>`.
#
# ```ruby
# class Xs                # represent a string of 'x's
#   include Comparable
#   attr :length
#   def initialize(n)
#     @length = n
#   end
#   def succ
#     Xs.new(@length + 1)
#   end
#   def <=>(other)
#     @length <=> other.length
#   end
#   def to_s
#     sprintf "%2d #{inspect}", @length
#   end
#   def inspect
#     'x' * @length
#   end
# end
# ```
#
# An example of using `Xs` to construct a range:
#
# ```ruby
# r = Xs.new(3)..Xs.new(6)   #=> xxx..xxxxxx
# r.to_a                     #=> [xxx, xxxx, xxxxx, xxxxxx]
# r.member?(Xs.new(5))       #=> true
# ```
class Range < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  # Returns `true` only if `obj` is a
  # [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html), has equivalent
  # begin and end items (by comparing them with `==`), and has the same
  # [`exclude_end?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-exclude_end-3F)
  # setting as the range.
  #
  # ```ruby
  # (0..2) == (0..2)            #=> true
  # (0..2) == Range.new(0,2)    #=> true
  # (0..2) == (0...2)           #=> false
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(obj); end

  # Returns `true` if `obj` is an element of the range, `false` otherwise.
  # Conveniently, `===` is the comparison operator used by `case` statements.
  #
  # ```ruby
  # case 79
  # when 1..50   then   print "low\n"
  # when 51..75  then   print "medium\n"
  # when 76..100 then   print "high\n"
  # end
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # high
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(obj); end

  # Returns the object that defines the beginning of the range.
  #
  # ```ruby
  # (1..10).begin   #=> 1
  # ```
  sig {returns(Elem)}
  def begin(); end

  # By using binary search, finds a value in range which meets the given
  # condition in O(log n) where n is the size of the range.
  #
  # You can use this method in two use cases: a find-minimum mode and a find-any
  # mode. In either case, the elements of the range must be monotone (or sorted)
  # with respect to the block.
  #
  # In find-minimum mode (this is a good choice for typical use case), the block
  # must return true or false, and there must be a value x so that:
  #
  # *   the block returns false for any value which is less than x, and
  # *   the block returns true for any value which is greater than or equal to
  #     x.
  #
  #
  # If x is within the range, this method returns the value x. Otherwise, it
  # returns nil.
  #
  # ```ruby
  # ary = [0, 4, 7, 10, 12]
  # (0...ary.size).bsearch {|i| ary[i] >= 4 } #=> 1
  # (0...ary.size).bsearch {|i| ary[i] >= 6 } #=> 2
  # (0...ary.size).bsearch {|i| ary[i] >= 8 } #=> 3
  # (0...ary.size).bsearch {|i| ary[i] >= 100 } #=> nil
  #
  # (0.0...Float::INFINITY).bsearch {|x| Math.log(x) >= 0 } #=> 1.0
  # ```
  #
  # In find-any mode (this behaves like libc's bsearch(3)), the block must
  # return a number, and there must be two values x and y (x <= y) so that:
  #
  # *   the block returns a positive number for v if v < x,
  # *   the block returns zero for v if x <= v < y, and
  # *   the block returns a negative number for v if y <= v.
  #
  #
  # This method returns any value which is within the intersection of the given
  # range and x...y (if any). If there is no value that satisfies the condition,
  # it returns nil.
  #
  # ```ruby
  # ary = [0, 100, 100, 100, 200]
  # (0..4).bsearch {|i| 100 - ary[i] } #=> 1, 2 or 3
  # (0..4).bsearch {|i| 300 - ary[i] } #=> nil
  # (0..4).bsearch {|i|  50 - ary[i] } #=> nil
  # ```
  #
  # You must not mix the two modes at a time; the block must always return
  # either true/false, or always return a number. It is undefined which value is
  # actually picked up at each iteration.
  sig do
    params(blk: T.proc.params(arg0: Elem).returns(T.any(Numeric, T::Boolean)))
      .returns(T.nilable(Elem))
  end
  def bsearch(&blk); end

  # Returns `true` if `obj` is between the begin and end of the range.
  #
  # This tests `begin <= obj <= end` when
  # [`exclude_end?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-exclude_end-3F)
  # is `false` and `begin <= obj < end` when
  # [`exclude_end?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-exclude_end-3F)
  # is `true`.
  #
  # If called with a [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html)
  # argument, returns `true` when the given range is covered by the receiver, by
  # comparing the begin and end values. If the argument can be treated as a
  # sequence, this method treats it that way. In the specific case of
  # `(a..b).cover?(c...d)` with `a <= c && b < d`, the end of the sequence must
  # be calculated, which may exhibit poor performance if `c` is non-numeric.
  # Returns `false` if the begin value of the range is larger than the end
  # value.
  #
  # ```ruby
  # ("a".."z").cover?("c")  #=> true
  # ("a".."z").cover?("5")  #=> false
  # ("a".."z").cover?("cc") #=> true
  # (1..5).cover?(2..3)     #=> true
  # (1..5).cover?(0..6)     #=> false
  # (1..5).cover?(1...6)    #=> true
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def cover?(obj); end

  # Iterates over the elements of range, passing each in turn to the block.
  #
  # The `each` method can only be used if the begin object of the range supports
  # the `succ` method. A
  # [`TypeError`](https://docs.ruby-lang.org/en/2.6.0/TypeError.html) is raised
  # if the object does not have `succ` method defined (like
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html)).
  #
  # If no block is given, an enumerator is returned instead.
  #
  # ```ruby
  # (10..15).each {|n| print n, ' ' }
  # # prints: 10 11 12 13 14 15
  #
  # (2.5..5).each {|n| print n, ' ' }
  # # raises: TypeError: can't iterate from Float
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(T::Enumerator[Elem])}
  def each(&blk); end

  # Returns the object that defines the end of the range.
  #
  # ```ruby
  # (1..10).end    #=> 10
  # (1...10).end   #=> 10
  # ```
  sig {returns(Elem)}
  def end(); end

  # Returns `true` if the range excludes its end value.
  #
  # ```ruby
  # (1..5).exclude_end?     #=> false
  # (1...5).exclude_end?    #=> true
  # ```
  sig {returns(T::Boolean)}
  def exclude_end?(); end

  # Returns the first object in the range, or an array of the first `n`
  # elements.
  #
  # ```ruby
  # (10..20).first     #=> 10
  # (10..20).first(3)  #=> [10, 11, 12]
  # ```
  sig {returns(Elem)}
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def first(n=T.unsafe(nil)); end

  # Compute a hash-code for this range. Two ranges with equal begin and end
  # points (using `eql?`), and the same
  # [`exclude_end?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-exclude_end-3F)
  # value will generate the same hash-code.
  #
  # See also Object#hash.
  sig {returns(Integer)}
  def hash(); end

  # Returns `true` if `obj` is an element of the range, `false` otherwise. If
  # begin and end are numeric, comparison is done according to the magnitude of
  # the values.
  #
  # ```ruby
  # ("a".."z").include?("g")   #=> true
  # ("a".."z").include?("A")   #=> false
  # ("a".."z").include?("cc")  #=> false
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def include?(obj); end

  sig do
    params(
        _begin: Elem,
        _end: Elem,
        exclude_end: T::Boolean,
    )
    .void
  end
  def initialize(_begin, _end, exclude_end=T.unsafe(nil)); end

  # Convert this range object to a printable form (using `inspect` to convert
  # the begin and end objects).
  sig {returns(String)}
  def inspect(); end

  # Returns the last object in the range, or an array of the last `n` elements.
  #
  # Note that with no arguments `last` will return the object that defines the
  # end of the range even if
  # [`exclude_end?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-exclude_end-3F)
  # is `true`.
  #
  # ```ruby
  # (10..20).last      #=> 20
  # (10...20).last     #=> 20
  # (10..20).last(3)   #=> [18, 19, 20]
  # (10...20).last(3)  #=> [17, 18, 19]
  # ```
  sig {returns(Elem)}
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def last(n=T.unsafe(nil)); end

  # Returns the maximum value in the range. Returns `nil` if the begin value of
  # the range larger than the end value. Returns `nil` if the begin value of an
  # exclusive range is equal to the end value.
  #
  # Can be given an optional block to override the default comparison method `a
  # <=> b`.
  #
  # ```ruby
  # (10..20).max    #=> 20
  # ```
  sig {returns(Elem)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(Elem)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def max(n=T.unsafe(nil), &blk); end

  # Returns the minimum value in the range. Returns `nil` if the begin value of
  # the range is larger than the end value. Returns `nil` if the begin value of
  # an exclusive range is equal to the end value.
  #
  # Can be given an optional block to override the default comparison method `a
  # <=> b`.
  #
  # ```ruby
  # (10..20).min    #=> 10
  # ```
  sig {returns(Elem)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(Elem)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min(n=T.unsafe(nil), &blk); end

  # Returns a two element array which contains the minimum and the
  # maximum value in the range.
  #
  # Can be given an optional block to override the default comparison
  # method `a <=> b`.
  sig {returns([T.nilable(Elem), T.nilable(Elem)])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns([T.nilable(Elem), T.nilable(Elem)])
  end
  def minmax(&blk); end

  # Returns the number of elements in the range. Both the begin and the end of
  # the [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html) must be
  # [`Numeric`](https://docs.ruby-lang.org/en/2.6.0/Numeric.html), otherwise nil
  # is returned.
  #
  # ```ruby
  # (10..20).size    #=> 11
  # ('a'..'z').size  #=> nil
  # (-Float::INFINITY..Float::INFINITY).size #=> Infinity
  # ```
  sig {returns(T.nilable(Integer))}
  def size(); end

  # Iterates over the range, passing each `n`th element to the block. If begin
  # and end are numeric, `n` is added for each iteration. Otherwise `step`
  # invokes `succ` to iterate through range elements.
  #
  # If no block is given, an enumerator is returned instead. Especially, the
  # enumerator is an
  # [`Enumerator::ArithmeticSequence`](https://docs.ruby-lang.org/en/2.6.0/Enumerator/ArithmeticSequence.html)
  # if begin and end of the range are numeric.
  #
  # ```ruby
  # range = Xs.new(1)..Xs.new(10)
  # range.step(2) {|x| puts x}
  # puts
  # range.step(3) {|x| puts x}
  # ```
  #
  # *produces:*
  #
  # ```
  #  1 x
  #  3 xxx
  #  5 xxxxx
  #  7 xxxxxxx
  #  9 xxxxxxxxx
  #
  #  1 x
  #  4 xxxx
  #  7 xxxxxxx
  # 10 xxxxxxxxxx
  # ```
  #
  # See [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html) for the
  # definition of class Xs.
  sig do
    params(
        n: T.any(Integer, Float, Rational),
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        n: T.any(Integer, Float, Rational),
    )
    .returns(T::Enumerator[Elem])
  end
  def step(n=T.unsafe(nil), &blk); end

  # Convert this range object to a printable form (using
  # [`to_s`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-to_s) to
  # convert the begin and end objects).
  sig {returns(String)}
  def to_s(); end

  # Returns `true` only if `obj` is a
  # [`Range`](https://docs.ruby-lang.org/en/2.6.0/Range.html), has equivalent
  # begin and end items (by comparing them with `eql?`), and has the same
  # [`exclude_end?`](https://docs.ruby-lang.org/en/2.6.0/Range.html#method-i-exclude_end-3F)
  # setting as the range.
  #
  # ```ruby
  # (0..2).eql?(0..2)            #=> true
  # (0..2).eql?(Range.new(0,2))  #=> true
  # (0..2).eql?(0...2)           #=> false
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(obj); end

  # Returns `true` if `obj` is an element of the range, `false` otherwise. If
  # begin and end are numeric, comparison is done according to the magnitude of
  # the values.
  #
  # ```ruby
  # ("a".."z").include?("g")   #=> true
  # ("a".."z").include?("A")   #=> false
  # ("a".."z").include?("cc")  #=> false
  # ```
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def member?(obj); end
end

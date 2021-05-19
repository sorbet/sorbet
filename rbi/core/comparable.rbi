# typed: __STDLIB_INTERNAL

# The [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html) mixin
# is used by classes whose objects may be ordered. The class must define the
# `<=>` operator, which compares the receiver against another object, returning
# a value less than 0, returning 0, or returning a value greater than 0,
# depending on whether the receiver is less than, equal to, or greater than the
# other object. If the other object is not comparable then the `<=>` operator
# should return `nil`.
# [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html) uses `<=>`
# to implement the conventional comparison operators (`<`, `<=`, `==`, `>=`, and
# `>`) and the method `between?`.
#
# ```ruby
# class SizeMatters
#   include Comparable
#   attr :str
#   def <=>(other)
#     str.size <=> other.str.size
#   end
#   def initialize(str)
#     @str = str
#   end
#   def inspect
#     @str
#   end
# end
#
# s1 = SizeMatters.new("Z")
# s2 = SizeMatters.new("YY")
# s3 = SizeMatters.new("XXX")
# s4 = SizeMatters.new("WWWW")
# s5 = SizeMatters.new("VVVVV")
#
# s1 < s2                       #=> true
# s4.between?(s1, s3)           #=> false
# s4.between?(s3, s5)           #=> true
# [ s3, s2, s5, s4, s1 ].sort   #=> [Z, YY, XXX, WWWW, VVVVV]
# ```
module Comparable
  # Compares two objects based on the receiver's `<=>` method, returning true if
  # it returns a value less than 0.
  sig { params(other: T.untyped).returns(T::Boolean) }
  def <(other); end

  # Compares two objects based on the receiver's `<=>` method, returning true if
  # it returns a value less than or equal to 0.
  sig { params(other: T.untyped).returns(T::Boolean) }
  def <=(other); end

  # Compares two objects based on the receiver's `<=>` method, returning true if
  # it returns 0. Also returns true if *obj* and *other* are the same object.
  sig { params(other: T.untyped).returns(T::Boolean) }
  def ==(other); end

  # Compares two objects based on the receiver's `<=>` method, returning true if
  # it returns a value greater than 0.
  sig { params(other: T.untyped).returns(T::Boolean) }
  def >(other); end

  # Compares two objects based on the receiver's `<=>` method, returning true if
  # it returns a value greater than or equal to 0.
  sig { params(other: T.untyped).returns(T::Boolean) }
  def >=(other); end

  # Returns `false` if *obj* `<=>` *min* is less than zero or if *obj* `<=>`
  # *max* is greater than zero, `true` otherwise.
  #
  # ```ruby
  # 3.between?(1, 5)               #=> true
  # 6.between?(1, 5)               #=> false
  # 'cat'.between?('ant', 'dog')   #=> true
  # 'gnu'.between?('ant', 'dog')   #=> false
  # ```
  sig { params(min: T.untyped, max: T.untyped).returns(T::Boolean) }
  def between?(min, max); end

  # In `(min, max)` form, returns *min* if *obj* `<=>` *min* is less than zero,
  # *max* if *obj* `<=>` *max* is greater than zero, and *obj* otherwise.
  #
  # ```ruby
  # 12.clamp(0, 100)         #=> 12
  # 523.clamp(0, 100)        #=> 100
  # -3.123.clamp(0, 100)     #=> 0
  #
  # 'd'.clamp('a', 'f')      #=> 'd'
  # 'z'.clamp('a', 'f')      #=> 'f'
  # ```
  #
  # In `(range)` form, returns *range.begin* if *obj* `<=>` *range.begin* is
  # less than zero, *range.end* if *obj* `<=>` *range.end* is greater than zero,
  # and *obj* otherwise.
  #
  # ```ruby
  # 12.clamp(0..100)         #=> 12
  # 523.clamp(0..100)        #=> 100
  # -3.123.clamp(0..100)     #=> 0
  #
  # 'd'.clamp('a'..'f')      #=> 'd'
  # 'z'.clamp('a'..'f')      #=> 'f'
  # ```
  #
  # If *range.begin* is `nil`, it is considered smaller than *obj*, and if
  # *range.end* is `nil`, it is considered greater than *obj*.
  #
  # ```ruby
  # -20.clamp(0..)           #=> 0
  # 523.clamp(..100)         #=> 100
  # ```
  #
  # When *range.end* is excluded and not `nil`, an exception is raised.
  #
  # ```ruby
  # 100.clamp(0...100)       # ArgumentError
  # ```
  sig { params(s: T::Range[T.untyped]).returns(T.untyped) }
  sig { params(s: T.untyped, e: T.untyped).returns(T.untyped) }
  def clamp(s, e); end
end

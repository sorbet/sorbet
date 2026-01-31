# typed: __STDLIB_INTERNAL

# [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) is the class
# from which all higher-level numeric classes should inherit.
#
# [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) allows
# instantiation of heap-allocated objects. Other core numeric classes such as
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) are implemented
# as immediates, which means that each
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) is a single
# immutable object which is always passed by value.
#
# ```ruby
# a = 1
# 1.object_id == a.object_id   #=> true
# ```
#
# There can only ever be one instance of the integer `1`, for example. Ruby
# ensures this by preventing instantiation. If duplication is attempted, the
# same instance is returned.
#
# ```ruby
# Integer.new(1)                   #=> NoMethodError: undefined method `new' for Integer:Class
# 1.dup                            #=> 1
# 1.object_id == 1.dup.object_id   #=> true
# ```
#
# For this reason, [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html)
# should be used when defining other numeric classes.
#
# Classes which inherit from
# [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) must implement
# `coerce`, which returns a two-member
# [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) containing an object
# that has been coerced into an instance of the new class and `self` (see
# [`coerce`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-coerce)).
#
# Inheriting classes should also implement arithmetic operator methods (`+`,
# `-`, `*` and `/`) and the `<=>` operator (see
# [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html)). These
# methods may rely on `coerce` to ensure interoperability with instances of
# other numeric classes.
#
# ```ruby
# class Tally < Numeric
#   def initialize(string)
#     @string = string
#   end
#
#   def to_s
#     @string
#   end
#
#   def to_i
#     @string.size
#   end
#
#   def coerce(other)
#     [self.class.new('|' * other.to_i), self]
#   end
#
#   def <=>(other)
#     to_i <=> other.to_i
#   end
#
#   def +(other)
#     self.class.new('|' * (to_i + other.to_i))
#   end
#
#   def -(other)
#     self.class.new('|' * (to_i - other.to_i))
#   end
#
#   def *(other)
#     self.class.new('|' * (to_i * other.to_i))
#   end
#
#   def /(other)
#     self.class.new('|' * (to_i / other.to_i))
#   end
# end
#
# tally = Tally.new('||')
# puts tally * 2            #=> "||||"
# puts tally > 1            #=> true
# ```
class Numeric < Object
  include Comparable

  # `x.modulo(y)` means `x-y*(x/y).floor`.
  #
  # Equivalent to `num.divmod(numeric)[1]`.
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def %(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def +(arg0); end

  # Unary Plus---Returns the receiver.
  sig {returns(Numeric)}
  def +@(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def -(arg0); end

  sig do
    params(
      arg0: Numeric,
    )
    .returns(Numeric)
  end
  def *(arg0); end

  sig do
    params(
      arg0: Numeric,
    )
    .returns(Numeric)
  end
  def /(arg0); end

  # Unary Minus---Returns the receiver, negated.
  sig {returns(Numeric)}
  def -@(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  # Returns zero if `number` equals `other`, otherwise returns `nil`.
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Integer)
  end
  sig { params(arg0: T.anything).returns(NilClass) }
  def <=>(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  # Returns the absolute value of `num`.
  #
  # ```ruby
  # 12.abs         #=> 12
  # (-34.56).abs   #=> 34.56
  # -34.56.abs     #=> 34.56
  # ```
  #
  # [`Numeric#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-magnitude)
  # is an alias for
  # [`Numeric#abs`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-abs).
  sig {returns(Numeric)}
  def abs(); end

  # Returns square of self.
  sig {returns(Numeric)}
  def abs2(); end

  # Returns 0 if the value is positive, pi otherwise.
  sig {returns(Numeric)}
  def angle(); end

  # Returns 0 if the value is positive, pi otherwise.
  sig {returns(Numeric)}
  def arg(); end

  # Returns the smallest number greater than or equal to `num` with a precision
  # of `ndigits` decimal digits (default: 0).
  #
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) implements
  # this by converting its value to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) and invoking
  # [`Float#ceil`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-ceil).
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  # If `numeric` is the same type as `num`, returns an array `[numeric, num]`.
  # Otherwise, returns an array with both `numeric` and `num` represented as
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) objects.
  #
  # This coercion mechanism is used by Ruby to handle mixed-type numeric
  # operations: it is intended to find a compatible common type between the two
  # operands of the operator.
  #
  # ```ruby
  # 1.coerce(2.5)   #=> [2.5, 1.0]
  # 1.2.coerce(3)   #=> [3.0, 1.2]
  # 1.coerce(2)     #=> [2, 1]
  # ```
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Numeric, Numeric])
  end
  def coerce(arg0); end

  # Returns self.
  sig {returns(Numeric)}
  def conj(); end

  # Returns self.
  sig {returns(Numeric)}
  def conjugate(); end

  # Returns the receiver. `freeze` cannot be `false`.
  def clone(*_); end

  # Returns the denominator (always positive).
  sig {returns(Integer)}
  def denominator(); end

  # Uses `/` to perform division, then converts the result to an integer.
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) does not
  # define the `/` operator; this is left to subclasses.
  #
  # Equivalent to `num.divmod(numeric)[0]`.
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Integer)
  end
  def div(arg0); end

  # Returns an array containing the quotient and modulus obtained by dividing
  # `num` by `numeric`.
  #
  # If `q, r = x.divmod(y)`, then
  #
  # ```ruby
  # q = floor(x/y)
  # x = q*y + r
  # ```
  #
  # The quotient is rounded toward negative infinity, as shown in the following
  # table:
  #
  # ```
  #  a    |  b  |  a.divmod(b)  |   a/b   | a.modulo(b) | a.remainder(b)
  # ------+-----+---------------+---------+-------------+---------------
  #  13   |  4  |   3,    1     |   3     |    1        |     1
  # ------+-----+---------------+---------+-------------+---------------
  #  13   | -4  |  -4,   -3     |  -4     |   -3        |     1
  # ------+-----+---------------+---------+-------------+---------------
  # -13   |  4  |  -4,    3     |  -4     |    3        |    -1
  # ------+-----+---------------+---------+-------------+---------------
  # -13   | -4  |   3,   -1     |   3     |   -1        |    -1
  # ------+-----+---------------+---------+-------------+---------------
  #  11.5 |  4  |   2,    3.5   |   2.875 |    3.5      |     3.5
  # ------+-----+---------------+---------+-------------+---------------
  #  11.5 | -4  |  -3,   -0.5   |  -2.875 |   -0.5      |     3.5
  # ------+-----+---------------+---------+-------------+---------------
  # -11.5 |  4  |  -3,    0.5   |  -2.875 |    0.5      |    -3.5
  # ------+-----+---------------+---------+-------------+---------------
  # -11.5 | -4  |   2,   -3.5   |   2.875 |   -3.5      |    -3.5
  # ```
  #
  # Examples
  #
  # ```ruby
  # 11.divmod(3)        #=> [3, 2]
  # 11.divmod(-3)       #=> [-4, -1]
  # 11.divmod(3.5)      #=> [3, 0.5]
  # (-11).divmod(3.5)   #=> [-4, 3.0]
  # 11.5.divmod(3.5)    #=> [3, 1.0]
  # ```
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Numeric, Numeric])
  end
  def divmod(arg0); end

  # Returns the receiver.
  def dup; end

  # Returns `true` if `num` and `numeric` are the same type and have equal
  # values. Contrast this with Numeric#==, which performs type conversions.
  #
  # ```ruby
  # 1 == 1.0        #=> true
  # 1.eql?(1.0)     #=> false
  # 1.0.eql?(1.0)   #=> true
  # ```
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  # Returns float division.
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def fdiv(arg0); end

  # Returns `true` if `num` is a finite number, otherwise returns `false`.
  sig {returns(T::Boolean)}
  def finite?; end

  # Returns the largest number less than or equal to `num` with a precision of
  # `ndigits` decimal digits (default: 0).
  #
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) implements
  # this by converting its value to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) and invoking
  # [`Float#floor`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-floor).
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

  # Returns the corresponding imaginary number. Not available for complex
  # numbers.
  #
  # ```ruby
  # -42.i  #=> (0-42i)
  # 2.0.i  #=> (0+2.0i)
  # ```
  sig {returns(Complex)}
  def i(); end

  # Returns zero.
  sig {returns(Numeric)}
  def imag(); end

  # Returns zero.
  sig {returns(Numeric)}
  def imaginary(); end

  # Returns `nil`, -1, or 1 depending on whether the value is finite,
  # `-Infinity`, or `+Infinity`.
  sig {returns(T.nilable(Integer))}
  def infinite?; end

  # Returns `true` if `num` is an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # ```ruby
  # 1.0.integer?   #=> false
  # 1.integer?     #=> true
  # ```
  sig {returns(T::Boolean)}
  def integer?(); end

  # Returns the absolute value of `num`.
  #
  # ```ruby
  # 12.abs         #=> 12
  # (-34.56).abs   #=> 34.56
  # -34.56.abs     #=> 34.56
  # ```
  #
  # [`Numeric#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-magnitude)
  # is an alias for
  # [`Numeric#abs`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-abs).
  sig {returns(Numeric)}
  def magnitude(); end

  # `x.modulo(y)` means `x-y*(x/y).floor`.
  #
  # Equivalent to `num.divmod(numeric)[1]`.
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float, Rational, BigDecimal))
  end
  def modulo(arg0); end

  # Returns `true` if `num` is less than 0.
  sig {returns(T::Boolean)}
  def negative?(); end

  # Returns `self` if `num` is not zero, `nil` otherwise.
  #
  # This behavior is useful when chaining comparisons:
  #
  # ```ruby
  # a = %w( z Bb bB bb BB a aA Aa AA A )
  # b = a.sort {|a,b| (a.downcase <=> b.downcase).nonzero? || a <=> b }
  # b   #=> ["A", "a", "AA", "Aa", "aA", "BB", "Bb", "bB", "bb", "z"]
  # ```
  sig {returns(T.nilable(T.self_type))}
  def nonzero?(); end

  # Returns the numerator.
  sig {returns(Integer)}
  def numerator(); end

  # Returns 0 if the value is positive, pi otherwise.
  sig {returns(Numeric)}
  def phase(); end

  # Returns an array; [num.abs, num.arg].
  sig {returns([Numeric, Numeric])}
  def polar(); end

  # Returns `true` if `num` is greater than 0.
  sig {returns(T::Boolean)}
  def positive?(); end

  # Returns the most exact division (rational for integers, float for floats).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def quo(arg0); end

  # Returns self.
  sig {returns(Numeric)}
  def real(); end

  # Returns `true` if `num` is a real number (i.e. not
  # [`Complex`](https://docs.ruby-lang.org/en/2.7.0/Complex.html)).
  sig {returns(Numeric)}
  def real?(); end

  # Returns an array; [num, 0].
  sig {returns([Numeric, Numeric])}
  def rect(); end

  # Returns an array; [num, 0].
  sig {returns([Numeric, Numeric])}
  def rectangular(); end

  # `x.remainder(y)` means `x-y*(x/y).truncate`.
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float, Rational, BigDecimal))
  end
  def remainder(arg0); end

  # Returns `num` rounded to the nearest value with a precision of `ndigits`
  # decimal digits (default: 0).
  #
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) implements
  # this by converting its value to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) and invoking
  # [`Float#round`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-round).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def round(arg0 = 0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(TypeError)
  end
  def singleton_method_added(arg0); end

  # Invokes the given block with the sequence of numbers starting at `num`,
  # incremented by `step` (defaulted to `1`) on each call.
  #
  # The loop finishes when the value to be passed to the block is greater than
  # `limit` (if `step` is positive) or less than `limit` (if `step` is
  # negative), where `limit` is defaulted to infinity.
  #
  # In the recommended keyword argument style, either or both of `step` and
  # `limit` (default infinity) can be omitted. In the fixed position argument
  # style, zero as a step (i.e. `num.step(limit, 0)`) is not allowed for
  # historical compatibility reasons.
  #
  # If all the arguments are integers, the loop operates using an integer
  # counter.
  #
  # If any of the arguments are floating point numbers, all are converted to
  # floats, and the loop is executed *floor(n + n\*Float::EPSILON) + 1* times,
  # where *n = (limit - num)/step*.
  #
  # Otherwise, the loop starts at `num`, uses either the less-than (`<`) or
  # greater-than (`>`) operator to compare the counter against `limit`, and
  # increments itself using the `+` operator.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead. Especially, the enumerator is an
  # [`Enumerator::ArithmeticSequence`](https://docs.ruby-lang.org/en/2.7.0/Enumerator/ArithmeticSequence.html)
  # if both `limit` and `step` are kind of
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) or `nil`.
  #
  # For example:
  #
  # ```ruby
  # p 1.step.take(4)
  # p 10.step(by: -1).take(4)
  # 3.step(to: 5) {|i| print i, " " }
  # 1.step(10, 2) {|i| print i, " " }
  # Math::E.step(to: Math::PI, by: 0.2) {|f| print f, " " }
  # ```
  #
  # Will produce:
  #
  # ```
  # [1, 2, 3, 4]
  # [10, 9, 8, 7]
  # 3 4 5
  # 1 3 5 7 9
  # 2.718281828459045 2.9182818284590453 3.118281828459045
  # ```
  sig do
    params(
        limit: T.nilable(Numeric),
        step: Numeric,
        blk: T.proc.params(arg0: Numeric).returns(BasicObject),
    )
    .returns(Numeric)
  end
  sig do
    params(
        limit: T.nilable(Numeric),
        step: Numeric,
    )
    .returns(T::Enumerator[Numeric])
  end
  def step(limit=nil, step=1, &blk); end

  # Returns the value as a complex.
  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(Integer)}
  def to_i(); end

  # Invokes the child class's `to_i` method to convert `num` to an integer.
  #
  # ```ruby
  # 1.0.class          #=> Float
  # 1.0.to_int.class   #=> Integer
  # 1.0.to_i.class     #=> Integer
  # ```
  sig {returns(Integer)}
  def to_int(); end

  # Returns `num` truncated (toward zero) to a precision of `ndigits` decimal
  # digits (default: 0).
  #
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) implements
  # this by converting its value to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) and invoking
  # [`Float#truncate`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-truncate).
  sig {returns(Integer)}
  sig do
    params(
      arg0: Integer
    )
    .returns(T.any(Integer, Float))
  end
  def truncate(arg0=T.unsafe(nil)); end

  # Returns `true` if `num` has a zero value.
  sig {returns(T::Boolean)}
  def zero?(); end
end

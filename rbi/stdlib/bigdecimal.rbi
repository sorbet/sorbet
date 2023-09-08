# typed: __STDLIB_INTERNAL

# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) provides
# arbitrary-precision floating point decimal arithmetic.
#
# ## Introduction
#
# Ruby provides built-in support for arbitrary precision integer arithmetic.
#
# For example:
#
# ```ruby
# 42**13  #=>   1265437718438866624512
# ```
#
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) provides
# similar support for very large or very accurate floating point numbers.
#
# Decimal arithmetic is also useful for general calculation, because it provides
# the correct answers people expect--whereas normal binary floating point
# arithmetic often introduces subtle errors because of the conversion between
# base 10 and base 2.
#
# For example, try:
#
# ```ruby
# sum = 0
# 10_000.times do
#   sum = sum + 0.0001
# end
# print sum #=> 0.9999999999999062
# ```
#
# and contrast with the output from:
#
# ```ruby
# require 'bigdecimal'
#
# sum = BigDecimal("0")
# 10_000.times do
#   sum = sum + BigDecimal("0.0001")
# end
# print sum #=> 0.1E1
# ```
#
# Similarly:
#
# ```ruby
# (BigDecimal("1.2") - BigDecimal("1.0")) == BigDecimal("0.2") #=> true
#
# (1.2 - 1.0) == 0.2 #=> false
# ```
#
# ## Special features of accurate decimal arithmetic
#
# Because [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) is
# more accurate than normal binary floating point arithmetic, it requires some
# special values.
#
# ### Infinity
#
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) sometimes
# needs to return infinity, for example if you divide a value by zero.
#
# ```ruby
# BigDecimal("1.0") / BigDecimal("0.0")  #=> Infinity
# BigDecimal("-1.0") / BigDecimal("0.0")  #=> -Infinity
# ```
#
# You can represent infinite numbers to
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) using the
# strings `'Infinity'`, `'+Infinity'` and `'-Infinity'` (case-sensitive)
#
# ### Not a Number
#
# When a computation results in an undefined value, the special value `NaN` (for
# 'not a number') is returned.
#
# Example:
#
# ```ruby
# BigDecimal("0.0") / BigDecimal("0.0") #=> NaN
# ```
#
# You can also create undefined values.
#
# NaN is never considered to be the same as any other value, even NaN itself:
#
# ```ruby
# n = BigDecimal('NaN')
# n == 0.0 #=> false
# n == n #=> false
# ```
#
# ### Positive and negative zero
#
# If a computation results in a value which is too small to be represented as a
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) within the
# currently specified limits of precision, zero must be returned.
#
# If the value which is too small to be represented is negative, a
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) value of
# negative zero is returned.
#
# ```ruby
# BigDecimal("1.0") / BigDecimal("-Infinity") #=> -0.0
# ```
#
# If the value is positive, a value of positive zero is returned.
#
# ```ruby
# BigDecimal("1.0") / BigDecimal("Infinity") #=> 0.0
# ```
#
# (See
# [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode)
# for how to specify limits of precision.)
#
# Note that `-0.0` and `0.0` are considered to be the same for the purposes of
# comparison.
#
# Note also that in mathematics, there is no particular concept of negative or
# positive zero; true mathematical zero has no sign.
#
# ## bigdecimal/util
#
# When you require `bigdecimal/util`, the
# [`to_d`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-to_d)
# method will be available on
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) and the
# native [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html),
# [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html),
# [`Rational`](https://docs.ruby-lang.org/en/2.7.0/Rational.html), and
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) classes:
#
# ```ruby
# require 'bigdecimal/util'
#
# 42.to_d         # => 0.42e2
# 0.5.to_d        # => 0.5e0
# (2/3r).to_d(3)  # => 0.667e0
# "0.5".to_d      # => 0.5e0
# ```
#
# ## License
#
# Copyright (C) 2002 by Shigeo Kobayashi <shigeo@tinyforest.gr.jp>.
#
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) is
# released under the Ruby and 2-clause BSD licenses. See LICENSE.txt for
# details.
#
# Maintained by mrkn <mrkn@mrkn.jp> and ruby-core members.
#
# Documented by zzak <zachary@zacharyscott.net>, mathew <meta@pobox.com>, and
# many other contributors.
class BigDecimal < Numeric
  # Base value used in internal calculations. On a 32 bit system,
  # [`BASE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#BASE) is 10000,
  # indicating that calculation is done in groups of 4 digits. (If it were
  # larger, BASE\*\*2 wouldn't fit in 32 bits, so you couldn't guarantee that
  # two groups could always be multiplied together without overflow.)
  BASE = T.let(T.unsafe(nil), Integer)
  # Determines whether overflow, underflow or zero divide result in an exception
  # being thrown. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  EXCEPTION_ALL = T.let(T.unsafe(nil), Integer)
  # Determines what happens when the result of a computation is infinity. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  EXCEPTION_INFINITY = T.let(T.unsafe(nil), Integer)

  # Determines what happens when the result of a computation is not a number (NaN). See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  EXCEPTION_NaN = T.let(T.unsafe(nil), Integer)

  # Determines what happens when the result of a computation is an overflow (a
  # result too large to be represented). See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  EXCEPTION_OVERFLOW = T.let(T.unsafe(nil), Integer)
  # Determines what happens when the result of a computation is an underflow (a
  # result too small to be represented). See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  EXCEPTION_UNDERFLOW = T.let(T.unsafe(nil), Integer)
  # Determines what happens when a division by zero is performed. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  EXCEPTION_ZERODIVIDE = T.let(T.unsafe(nil), Integer)
  # Positive infinity value.
  INFINITY = T.let(T.unsafe(nil), BigDecimal)
  # 'Not a Number' value.
  NAN = T.let(T.unsafe(nil), BigDecimal)
  # Round towards +Infinity. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_CEILING = T.let(T.unsafe(nil), Integer)
  # Indicates that values should be rounded towards zero. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_DOWN = T.let(T.unsafe(nil), Integer)
  # Round towards -Infinity. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_FLOOR = T.let(T.unsafe(nil), Integer)
  # Indicates that digits >= 6 should be rounded up, others rounded down. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_HALF_DOWN = T.let(T.unsafe(nil), Integer)
  # Round towards the even neighbor. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_HALF_EVEN = T.let(T.unsafe(nil), Integer)
  # Indicates that digits >= 5 should be rounded up, others rounded down. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_HALF_UP = T.let(T.unsafe(nil), Integer)
  # Determines what happens when a result must be rounded in order to fit in the
  # appropriate number of significant digits. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_MODE = T.let(T.unsafe(nil), Integer)
  # Indicates that values should be rounded away from zero. See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  ROUND_UP = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is negative and finite. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_NEGATIVE_FINITE = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is negative and infinite. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_NEGATIVE_INFINITE = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is -0. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_NEGATIVE_ZERO = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is positive and finite. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_POSITIVE_FINITE = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is positive and infinite. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_POSITIVE_INFINITE = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is +0. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_POSITIVE_ZERO = T.let(T.unsafe(nil), Integer)
  # Indicates that a value is not a number. See
  # [`BigDecimal.sign`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-sign).
  SIGN_NaN = T.let(T.unsafe(nil), Integer)

  ### internal method, here for completeness
  # Internal method used to provide marshalling support. See the
  # [`Marshal`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html) module.
  sig {params(p1: T.untyped).returns(T.untyped)}
  def self._load(p1); end

  # The
  # [`BigDecimal.double_fig`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-double_fig)
  # class method returns the number of digits a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) number is allowed
  # to have. The result depends upon the CPU and OS in use.
  sig {returns(Integer)}
  def self.double_fig; end

  # Limit the number of significant digits in newly created
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) numbers
  # to the specified value. Rounding is performed as necessary, as specified by
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  #
  # A limit of 0, the default, means no upper limit.
  #
  # The limit specified by this method takes less priority over any limit
  # specified to instance methods such as ceil, floor, truncate, or round.
  sig {params(digits: Integer).returns(Integer)}
  def self.limit(digits=0); end

  ### `mode` can be one of:
  ###   BigDecimal::EXCEPTION_ALL
  ###   BigDecimal::EXCEPTION_INFINITY
  ###   BigDecimal::EXCEPTION_NaN
  ###   BigDecimal::EXCEPTION_OVERFLOW
  ###   BigDecimal::EXCEPTION_UNDERFLOW
  ###   BigDecimal::EXCEPTION_ZERODIVIDE
  ###   BigDecimal::ROUND_MODE
  ### @see https://ruby-doc.org/stdlib-2.6.3/libdoc/bigdecimal/rdoc/BigDecimal.html#method-c-mode
  # Controls handling of arithmetic exceptions and rounding. If no value is
  # supplied, the current value is returned.
  #
  # Six values of the mode parameter control the handling of arithmetic
  # exceptions:
  #
  # [`BigDecimal::EXCEPTION_NaN`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_NaN)
  # [`BigDecimal::EXCEPTION_INFINITY`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_INFINITY)
  # [`BigDecimal::EXCEPTION_UNDERFLOW`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_UNDERFLOW)
  # [`BigDecimal::EXCEPTION_OVERFLOW`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_OVERFLOW)
  # [`BigDecimal::EXCEPTION_ZERODIVIDE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_ZERODIVIDE)
  # [`BigDecimal::EXCEPTION_ALL`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_ALL)
  #
  # For each mode parameter above, if the value set is false, computation
  # continues after an arithmetic exception of the appropriate type. When
  # computation continues, results are as follows:
  #
  # [`EXCEPTION_NaN`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_NaN)
  # :   NaN
  # [`EXCEPTION_INFINITY`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_INFINITY)
  # :   +Infinity or -Infinity
  # [`EXCEPTION_UNDERFLOW`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_UNDERFLOW)
  # :   0
  # [`EXCEPTION_OVERFLOW`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_OVERFLOW)
  # :   +Infinity or -Infinity
  # [`EXCEPTION_ZERODIVIDE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#EXCEPTION_ZERODIVIDE)
  # :   +Infinity or -Infinity
  #
  #
  # One value of the mode parameter controls the rounding of numeric values:
  # [`BigDecimal::ROUND_MODE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_MODE).
  # The values it can take are:
  #
  # [`ROUND_UP`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_UP), :up
  # :   round away from zero
  # [`ROUND_DOWN`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_DOWN), :down, :truncate
  # :   round towards zero (truncate)
  # [`ROUND_HALF_UP`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_HALF_UP), :half\_up, :default
  # :   round towards the nearest neighbor, unless both neighbors are
  #     equidistant, in which case round away from zero. (default)
  # [`ROUND_HALF_DOWN`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_HALF_DOWN), :half\_down
  # :   round towards the nearest neighbor, unless both neighbors are
  #     equidistant, in which case round towards zero.
  # [`ROUND_HALF_EVEN`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_HALF_EVEN), :half\_even, :banker
  # :   round towards the nearest neighbor, unless both neighbors are
  #     equidistant, in which case round towards the even neighbor (Banker's
  #     rounding)
  # [`ROUND_CEILING`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_CEILING), :ceiling, :ceil
  # :   round towards positive infinity (ceil)
  # [`ROUND_FLOOR`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#ROUND_FLOOR), :floor
  # :   round towards negative infinity (floor)
  sig do
    params(
      mode: Integer,
      value: T.any(FalseClass, Integer, Symbol, TrueClass)
    )
    .returns(Integer)
  end
  def self.mode(mode, value=T.unsafe(nil)); end

  # Execute the provided block, but preserve the exception mode
  #
  # ```ruby
  # BigDecimal.save_exception_mode do
  #   BigDecimal.mode(BigDecimal::EXCEPTION_OVERFLOW, false)
  #   BigDecimal.mode(BigDecimal::EXCEPTION_NaN, false)
  #
  #   BigDecimal(BigDecimal('Infinity'))
  #   BigDecimal(BigDecimal('-Infinity'))
  #   BigDecimal(BigDecimal('NaN'))
  # end
  # ```
  #
  # For use with the BigDecimal::EXCEPTION\_\*
  #
  # See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode)
  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def self.save_exception_mode(&blk); end

  # Execute the provided block, but preserve the precision limit
  #
  # ```ruby
  # BigDecimal.limit(100)
  # puts BigDecimal.limit
  # BigDecimal.save_limit do
  #     BigDecimal.limit(200)
  #     puts BigDecimal.limit
  # end
  # puts BigDecimal.limit
  # ```
  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def self.save_limit(&blk); end

  # Execute the provided block, but preserve the rounding mode
  #
  # ```ruby
  # BigDecimal.save_rounding_mode do
  #   BigDecimal.mode(BigDecimal::ROUND_MODE, :up)
  #   puts BigDecimal.mode(BigDecimal::ROUND_MODE)
  # end
  # ```
  #
  # For use with the BigDecimal::ROUND\_\*
  #
  # See
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode)
  sig do
    type_parameters(:U)
    .params(blk: T.proc.returns(T.type_parameter(:U)))
    .returns(T.type_parameter(:U))
  end
  def self.save_rounding_mode(&blk); end

  sig do
    params(
      initial: T.any(Integer, Float, Rational, BigDecimal, String),
      digits: Integer,
    )
    .void
  end
  def initialize(initial, digits=0); end

  # Returns the modulus from dividing by b.
  #
  # See
  # [`BigDecimal#divmod`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-divmod).
  sig do
    params(
        arg0: Numeric,
    )
    .returns(BigDecimal)
  end
  def %(arg0); end

  # Multiply by the specified value.
  #
  # e.g.
  #
  # ```ruby
  # c = a.mult(b,n)
  # c = a * b
  # ```
  #
  # digits
  # :   If specified and less than the number of significant digits of the
  #     result, the result is rounded to that number of digits, according to
  #     [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def *(arg0); end

  # Returns the value raised to the power of n.
  #
  # See
  # [`BigDecimal#power`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-power).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  def **(arg0); end

  # Add the specified value.
  #
  # e.g.
  #
  # ```ruby
  # c = a.add(b,n)
  # c = a + b
  # ```
  #
  # digits
  # :   If specified and less than the number of significant digits of the
  #     result, the result is rounded to that number of digits, according to
  #     [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def +(arg0); end

  # Return self.
  #
  # ```ruby
  # +BigDecimal('5')  #=> 0.5e1
  # ```
  sig {returns(BigDecimal)}
  def +@(); end

  # Subtract the specified value.
  #
  # e.g.
  #
  # ```ruby
  # c = a - b
  # ```
  #
  # The precision of the result value depends on the type of `b`.
  #
  # If `b` is a [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html), the
  # precision of the result is Float::DIG+1.
  #
  # If `b` is a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html), the
  # precision of the result is `b`'s precision of internal representation from
  # platform. So, it's return value is platform dependent.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def -(arg0); end

  # Return the negation of self.
  #
  # ```ruby
  # -BigDecimal('5')  #=> -0.5e1
  # ```
  sig {returns(BigDecimal)}
  def -@(); end

  # Divide by the specified value.
  #
  # See
  # [`BigDecimal#div`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-div).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def /(arg0); end

  # Returns true if a is less than b.
  #
  # Values may be coerced to perform the comparison (see ==,
  # [`BigDecimal#coerce`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-coerce)).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  # Returns true if a is less than or equal to b.
  #
  # Values may be coerced to perform the comparison (see ==,
  # [`BigDecimal#coerce`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-coerce)).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  # The comparison operator. a <=> b is 0 if a == b, 1 if a > b, -1 if a < b.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Integer)
  end
  def <=>(arg0); end

  # Tests for value equality; returns true if the values are equal.
  #
  # The == and === operators and the eql? method have the same implementation
  # for [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # Values may be coerced to perform the comparison:
  #
  # ```ruby
  # BigDecimal('1.0') == 1.0  #=> true
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Tests for value equality; returns true if the values are equal.
  #
  # The == and === operators and the eql? method have the same implementation
  # for [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # Values may be coerced to perform the comparison:
  #
  # ```ruby
  # BigDecimal('1.0') == 1.0  #=> true
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  # Returns true if a is greater than b.
  #
  # Values may be coerced to perform the comparison (see ==,
  # [`BigDecimal#coerce`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-coerce)).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  # Returns true if a is greater than or equal to b.
  #
  # Values may be coerced to perform the comparison (see ==,
  # [`BigDecimal#coerce`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-coerce))
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  # [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) used to provide
  # marshalling support.
  #
  # ```ruby
  # inf = BigDecimal('Infinity')
  #   #=> Infinity
  # BigDecimal._load(inf._dump)
  #   #=> Infinity
  # ```
  #
  # See the [`Marshal`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html)
  # module.
  sig {returns(String)}
  def _dump(); end

  # Returns the absolute value, as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # BigDecimal('5').abs  #=> 0.5e1
  # BigDecimal('-3').abs #=> 0.3e1
  # ```
  sig {returns(BigDecimal)}
  def abs(); end

  sig {returns(BigDecimal)}
  def abs2(); end

  # Add the specified value.
  #
  # e.g.
  #
  # ```ruby
  # c = a.add(b,n)
  # c = a + b
  # ```
  #
  # digits
  # :   If specified and less than the number of significant digits of the
  #     result, the result is rounded to that number of digits, according to
  #     [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def add(arg0, arg1); end

  sig {returns(Numeric)}
  def angle(); end

  sig {returns(Numeric)}
  def arg(); end

  # Return the smallest integer greater than or equal to the value, as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # BigDecimal('3.14159').ceil #=> 4
  # BigDecimal('-9.1').ceil #=> -9
  # ```
  #
  # If n is specified and positive, the fractional part of the result has no
  # more than that many digits.
  #
  # If n is specified and negative, at least that many digits to the left of the
  # decimal point will be 0 in the result.
  #
  # ```ruby
  # BigDecimal('3.14159').ceil(3) #=> 3.142
  # BigDecimal('13345.234').ceil(-2) #=> 13400.0
  # ```
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(BigDecimal)
  end
  def ceil(digits=0); end

  # The coerce method provides support for Ruby type coercion. It is not enabled
  # by default.
  #
  # This means that binary operations like + \* / or - can often be performed on
  # a [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) and an
  # object of another type, if the other object can be coerced into a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) value.
  #
  # e.g.
  #
  # ```ruby
  # a = BigDecimal("1.0")
  # b = a / 2.0 #=> 0.5
  # ```
  #
  # Note that coercing a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) to a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) is not
  # supported by default; it requires a special compile-time option when
  # building Ruby.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([BigDecimal, BigDecimal])
  end
  def coerce(arg0); end

  sig {returns(BigDecimal)}
  def conj(); end

  sig {returns(BigDecimal)}
  def conjugate(); end

  sig {returns(Integer)}
  def denominator(); end

  # Divide by the specified value.
  #
  # digits
  # :   If specified and less than the number of significant digits of the
  #     result, the result is rounded to that number of digits, according to
  #     [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  #
  #     If digits is 0, the result is the same as for the / operator or
  #     [`quo`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-quo).
  #
  #     If digits is not specified, the result is an integer, by analogy with
  #     [`Float#div`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-div);
  #     see also
  #     [`BigDecimal#divmod`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-divmod).
  #
  #
  # Examples:
  #
  # ```ruby
  # a = BigDecimal("4")
  # b = BigDecimal("3")
  #
  # a.div(b, 3)  # => 0.133e1
  #
  # a.div(b, 0)  # => 0.1333333333333333333e1
  # a / b        # => 0.1333333333333333333e1
  # a.quo(b)     # => 0.1333333333333333333e1
  #
  # a.div(b)     # => 1
  # ```
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Integer)
  end
  def div(arg0); end

  # Divides by the specified value, and returns the quotient and modulus as
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) numbers.
  # The quotient is rounded towards negative infinity.
  #
  # For example:
  #
  # ```ruby
  # require 'bigdecimal'
  #
  # a = BigDecimal("42")
  # b = BigDecimal("9")
  #
  # q, m = a.divmod(b)
  #
  # c = q * b + m
  #
  # a == c  #=> true
  # ```
  #
  # The quotient q is (a/b).floor, and the modulus is the amount that must be
  # added to q \* b to get a.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def divmod(arg0); end

  # Tests for value equality; returns true if the values are equal.
  #
  # The == and === operators and the eql? method have the same implementation
  # for [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # Values may be coerced to perform the comparison:
  #
  # ```ruby
  # BigDecimal('1.0') == 1.0  #=> true
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def equal?(arg0); end

  # Returns the exponent of the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) number,
  # as an [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # If the number can be represented as 0.xxxxxx\*10\*\*n where xxxxxx is a
  # string of digits with no leading zeros, then n is the exponent.
  sig {returns(Integer)}
  def exponent(); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational),
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T.any(Float, BigDecimal))
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def fdiv(arg0); end

  # Returns True if the value is finite (not NaN or infinite).
  sig {returns(T::Boolean)}
  def finite?(); end

  # Return the integer part of the number, as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  sig {returns(BigDecimal)}
  def fix(); end

  # Return the largest integer less than or equal to the value, as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # BigDecimal('3.14159').floor #=> 3
  # BigDecimal('-9.1').floor #=> -10
  # ```
  #
  # If n is specified and positive, the fractional part of the result has no
  # more than that many digits.
  #
  # If n is specified and negative, at least that many digits to the left of the
  # decimal point will be 0 in the result.
  #
  # ```ruby
  # BigDecimal('3.14159').floor(3) #=> 3.141
  # BigDecimal('13345.234').floor(-2) #=> 13300.0
  # ```
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(BigDecimal)
  end
  def floor(digits=0); end

  # Return the fractional part of the number, as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  sig {returns(BigDecimal)}
  def frac(); end

  # Creates a hash for this
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # Two BigDecimals with equal sign, fractional part and exponent have the same
  # hash.
  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  # Returns nil, -1, or +1 depending on whether the value is finite, -Infinity,
  # or +Infinity.
  sig {returns(T.nilable(Integer))}
  def infinite?(); end

  # Returns a string representation of self.
  #
  # ```ruby
  # BigDecimal("1234.5678").inspect
  #   #=> "0.12345678e4"
  # ```
  sig {returns(String)}
  def inspect(); end

  sig {returns(BigDecimal)}
  def magnitude(); end

  # Returns the modulus from dividing by b.
  #
  # See
  # [`BigDecimal#divmod`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-divmod).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  def modulo(arg0); end

  # Multiply by the specified value.
  #
  # e.g.
  #
  # ```ruby
  # c = a.mult(b,n)
  # c = a * b
  # ```
  #
  # digits
  # :   If specified and less than the number of significant digits of the
  #     result, the result is rounded to that number of digits, according to
  #     [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def mult(arg0, arg1); end

  # Returns True if the value is Not a Number.
  sig {returns(T::Boolean)}
  def nan?(); end

  # Returns self if the value is non-zero, nil otherwise.
  sig {returns(Object)}
  def nonzero?(); end

  sig {returns(Integer)}
  def numerator(); end

  sig {returns(Numeric)}
  def phase(); end

  # Returns the value raised to the power of n.
  #
  # Note that n must be an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # Also available as the operator \*\*.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  def power(arg0); end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of two
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) values.
  #
  # The first value is the current number of significant digits in the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html). The
  # second value is the maximum number of significant digits for the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # BigDecimal('5').precs #=> [9, 18]
  # ```
  sig {returns([Integer, Integer])}
  def precs(); end

  # Divide by the specified value.
  #
  # See
  # [`BigDecimal#div`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-div).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def quo(arg0); end

  sig {returns(BigDecimal)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  # Returns the remainder from dividing by the value.
  #
  # x.remainder(y) means x-y\*(x/y).truncate
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  def remainder(arg0); end

  # Round to the nearest integer (by default), returning the result as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) if n is
  # specified, or as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) if it isn't.
  #
  # ```ruby
  # BigDecimal('3.14159').round #=> 3
  # BigDecimal('8.7').round #=> 9
  # BigDecimal('-9.9').round #=> -10
  #
  # BigDecimal('3.14159').round(2).class.name #=> "BigDecimal"
  # BigDecimal('3.14159').round.class.name #=> "Integer"
  # ```
  #
  # If n is specified and positive, the fractional part of the result has no
  # more than that many digits.
  #
  # If n is specified and negative, at least that many digits to the left of the
  # decimal point will be 0 in the result.
  #
  # ```ruby
  # BigDecimal('3.14159').round(3) #=> 3.142
  # BigDecimal('13345.234').round(-2) #=> 13300.0
  # ```
  #
  # The value of the optional mode argument can be used to determine how
  # rounding is performed; see
  # [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  sig {returns(Integer)}
  sig do
    params(
        n: Integer,
        mode: T.any(Integer, Symbol),
    )
    .returns(BigDecimal)
  end
  def round(n=0, mode=T.unsafe(nil)); end

  # Returns the sign of the value.
  #
  # Returns a positive value if > 0, a negative value if < 0, and a zero if ==
  # 0.
  #
  # The specific value returned indicates the type and sign of the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html), as
  # follows:
  #
  # [`BigDecimal::SIGN_NaN`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_NaN)
  # :   value is Not a Number
  # [`BigDecimal::SIGN_POSITIVE_ZERO`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_POSITIVE_ZERO)
  # :   value is +0
  # [`BigDecimal::SIGN_NEGATIVE_ZERO`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_NEGATIVE_ZERO)
  # :   value is -0
  # [`BigDecimal::SIGN_POSITIVE_INFINITE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_POSITIVE_INFINITE)
  # :   value is +Infinity
  # [`BigDecimal::SIGN_NEGATIVE_INFINITE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_NEGATIVE_INFINITE)
  # :   value is -Infinity
  # [`BigDecimal::SIGN_POSITIVE_FINITE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_POSITIVE_FINITE)
  # :   value is positive
  # [`BigDecimal::SIGN_NEGATIVE_FINITE`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#SIGN_NEGATIVE_FINITE)
  # :   value is negative
  sig {returns(Integer)}
  def sign(); end

  # Splits a [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html)
  # number into four parts, returned as an array of values.
  #
  # The first value represents the sign of the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html), and is
  # -1 or 1, or 0 if the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) is Not a
  # Number.
  #
  # The second value is a string representing the significant digits of the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html), with no
  # leading zeros.
  #
  # The third value is the base used for arithmetic (currently always 10) as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # The fourth value is an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) exponent.
  #
  # If the [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html)
  # can be represented as 0.xxxxxx\*10\*\*n, then xxxxxx is the string of
  # significant digits with no leading zeros, and n is the exponent.
  #
  # From these values, you can translate a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) to a
  # float as follows:
  #
  # ```ruby
  # sign, significant_digits, base, exponent = a.split
  # f = sign * "0.#{significant_digits}".to_f * (base ** exponent)
  # ```
  #
  # (Note that the
  # [`to_f`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-i-to_f)
  # method is provided as a more convenient way to translate a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html).)
  sig {returns([Integer, String, Integer, Integer])}
  def split(); end

  # Returns the square root of the value.
  #
  # Result has at least n significant digits.
  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  def sqrt(arg0); end

  # Subtract the specified value.
  #
  # e.g.
  #
  # ```ruby
  # c = a.sub(b,n)
  # ```
  #
  # digits
  # :   If specified and less than the number of significant digits of the
  #     result, the result is rounded to that number of digits, according to
  #     [`BigDecimal.mode`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html#method-c-mode).
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def sub(arg0, arg1); end

  sig {returns(Complex)}
  def to_c(); end

  # Returns a new [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html)
  # object having approximately the same value as the
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) number.
  # Normal accuracy limits and built-in errors of binary
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) arithmetic apply.
  sig {returns(Float)}
  def to_f(); end

  # Returns self.
  #
  # ```ruby
  # require 'bigdecimal/util'
  #
  # d = BigDecimal("3.14")
  # d.to_d                       # => 0.314e1
  # ```
  sig { returns(BigDecimal) }
  def to_d; end

  # Returns the value as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # If the [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html)
  # is infinity or NaN, raises
  # [`FloatDomainError`](https://docs.ruby-lang.org/en/2.7.0/FloatDomainError.html).
  sig {returns(Integer)}
  def to_i(); end

  # Returns the value as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # If the [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html)
  # is infinity or NaN, raises
  # [`FloatDomainError`](https://docs.ruby-lang.org/en/2.7.0/FloatDomainError.html).
  sig {returns(Integer)}
  def to_int(); end

  # Converts a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html) to a
  # [`Rational`](https://docs.ruby-lang.org/en/2.7.0/Rational.html).
  sig {returns(Rational)}
  def to_r(); end

  # Converts the value to a string.
  #
  # The default format looks like  0.xxxxEnn.
  #
  # The optional parameter s consists of either an integer; or an optional '+'
  # or ' ', followed by an optional number, followed by an optional 'E' or 'F'.
  #
  # If there is a '+' at the start of s, positive values are returned with a
  # leading '+'.
  #
  # A space at the start of s returns positive values with a leading space.
  #
  # If s contains a number, a space is inserted after each group of that many
  # fractional digits.
  #
  # If s ends with an 'E', engineering notation (0.xxxxEnn) is used.
  #
  # If s ends with an 'F', conventional floating point notation is used.
  #
  # Examples:
  #
  # ```ruby
  # BigDecimal('-123.45678901234567890').to_s('5F')
  #   #=> '-123.45678 90123 45678 9'
  #
  # BigDecimal('123.45678901234567890').to_s('+8F')
  #   #=> '+123.45678901 23456789'
  #
  # BigDecimal('123.45678901234567890').to_s(' F')
  #   #=> ' 123.4567890123456789'
  # ```
  sig {params(s: T.any(Integer, String)).returns(String)}
  def to_s(s=''); end

  # Truncate to the nearest integer (by default), returning the result as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # BigDecimal('3.14159').truncate #=> 3
  # BigDecimal('8.7').truncate #=> 8
  # BigDecimal('-9.9').truncate #=> -9
  # ```
  #
  # If n is specified and positive, the fractional part of the result has no
  # more than that many digits.
  #
  # If n is specified and negative, at least that many digits to the left of the
  # decimal point will be 0 in the result.
  #
  # ```ruby
  # BigDecimal('3.14159').truncate(3) #=> 3.141
  # BigDecimal('13345.234').truncate(-2) #=> 13300.0
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  def truncate(arg0=T.unsafe(nil)); end

  # Returns True if the value is zero.
  sig {returns(T::Boolean)}
  def zero?(); end
end

# require 'bigdecimal/jacobian'
#
# Provides methods to compute the
# [`Jacobian`](https://docs.ruby-lang.org/en/2.7.0/Jacobian.html) matrix of a
# set of equations at a point x. In the methods below:
#
# f is an [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) which is
# used to compute the
# [`Jacobian`](https://docs.ruby-lang.org/en/2.7.0/Jacobian.html) matrix of the
# equations. It must provide the following methods:
#
# f.values(x)
# :   returns the values of all functions at x
#
# f.zero
# :   returns 0.0
# f.one
# :   returns 1.0
# f.two
# :   returns 2.0
# f.ten
# :   returns 10.0
#
# f.eps
# :   returns the convergence criterion (epsilon value) used to determine
#     whether two values are considered equal. If |a-b| < epsilon, the two
#     values are considered equal.
#
#
# x is the point at which to compute the
# [`Jacobian`](https://docs.ruby-lang.org/en/2.7.0/Jacobian.html).
#
# fx is f.values(x).
module Jacobian

  # Computes the derivative of `f` at `x`. `fx` is the value of `f` at `x`.
  def dfdxi(f, fx, x, i); end

  # Computes the derivative of `f` at `x`. `fx` is the value of `f` at `x`.
  def self.dfdxi(f, fx, x, i); end

  # Determines the equality of two numbers by comparing to zero, or using the epsilon value
  def isEqual(a, b, zero = 0.0, e = _); end

  # Determines the equality of two numbers by comparing to zero, or using the epsilon value
  def self.isEqual(a, b, zero = 0.0, e = _); end

  # Computes the
  # [`Jacobian`](https://docs.ruby-lang.org/en/2.6.0/Jacobian.html)
  # of `f` at `x`. `fx` is the value of `f` at `x`.
  def jacobian(f, fx, x); end

  # Computes the
  # [`Jacobian`](https://docs.ruby-lang.org/en/2.6.0/Jacobian.html)
  # of `f` at `x`. `fx` is the value of `f` at `x`.
  def self.jacobian(f, fx, x); end
end

# Solves a\*x = b for x, using LU decomposition.
module LUSolve
  # Performs LU decomposition of the `n` by `n` matrix `a`.
  def ludecomp(a, n, zero = 0, one = 1); end

  # Performs LU decomposition of the `n` by `n` matrix `a`.
  def self.ludecomp(a, n, zero = 0, one = 1); end

  # Solves `a*x = b` for `x`, using LU decomposition.
  #
  # `a` is a matrix, `b` is a constant vector, `x` is the solution vector.
  #
  # `ps` is the pivot, a vector which indicates the permutation of rows
  # performed during LU decomposition.
  def lusolve(a, b, ps, zero = 0.0); end

  # Solves `a*x = b` for `x`, using LU decomposition.
  #
  # `a` is a matrix, `b` is a constant vector, `x` is the solution vector.
  #
  # `ps` is the pivot, a vector which indicates the permutation of rows
  # performed during LU decomposition.
  def self.lusolve(a, b, ps, zero = 0.0); end
end

# newton.rb
#
# Solves the nonlinear algebraic equation system f = 0 by Newton's method. This
# program is not dependent on
# [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
#
# To call:
#
# ```
#   n = nlsolve(f,x)
# where n is the number of iterations required,
#       x is the initial value vector
#       f is an Object which is used to compute the values of the equations to be solved.
# ```
#
# It must provide the following methods:
#
# f.values(x)
# :   returns the values of all functions at x
#
# f.zero
# :   returns 0.0
# f.one
# :   returns 1.0
# f.two
# :   returns 2.0
# f.ten
# :   returns 10.0
#
# f.eps
# :   returns the convergence criterion (epsilon value) used to determine
#     whether two values are considered equal. If |a-b| < epsilon, the two
#     values are considered equal.
#
#
# On exit, x is the solution vector.
module Newton
  include(::Jacobian)
  include(::LUSolve)

  def nlsolve(f, x); end

  def self.nlsolve(f, x); end

  def norm(fv, zero = _); end

  def self.norm(fv, zero = _); end
end

# [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) objects represent
# inexact real numbers using the native architecture's double-precision floating
# point representation.
#
# Floating point has a different arithmetic and is an inexact number. So you
# should know its esoteric system. See following:
#
# *   http://docs.sun.com/source/806-3568/ncg\_goldberg.html
# *   https://github.com/rdp/ruby\_tutorials\_core/wiki/Ruby-Talk-FAQ#floats\_imprecise
# *   http://en.wikipedia.org/wiki/Floating\_point#Accuracy\_problems
class Float < ::Numeric
  # Returns the value of `float` as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html). The
  # `precision` parameter is used to determine the number of significant digits
  # for the result (the default is
  # [`Float::DIG`](https://docs.ruby-lang.org/en/2.7.0/Float.html#DIG)).
  #
  # ```ruby
  # require 'bigdecimal'
  # require 'bigdecimal/util'
  #
  # 0.5.to_d         # => 0.5e0
  # 1.234.to_d(2)    # => 0.12e1
  # ```
  #
  # See also
  # [`BigDecimal::new`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-c-new).
  sig { params(precision: Integer).returns(BigDecimal) }
  def to_d(precision = _); end
end

# Holds [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) values.
# You cannot add a singleton method to an
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) object, any
# attempt to do so will raise a
# [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html).
class Integer < ::Numeric
  # Returns the value of `int` as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # require 'bigdecimal'
  # require 'bigdecimal/util'
  #
  # 42.to_d   # => 0.42e2
  # ```
  #
  # See also
  # [`BigDecimal::new`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-c-new).
  sig { returns(BigDecimal) }
  def to_d; end
end

# A rational number can be represented as a pair of integer numbers: a/b (b>0),
# where a is the numerator and b is the denominator.
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) a equals
# rational a/1 mathematically.
#
# In Ruby, you can create rational objects with the Kernel#Rational,
# [`to_r`](https://docs.ruby-lang.org/en/2.7.0/Rational.html#method-i-to_r), or
# rationalize methods or by suffixing `r` to a literal. The return values will
# be irreducible fractions.
#
# ```ruby
# Rational(1)      #=> (1/1)
# Rational(2, 3)   #=> (2/3)
# Rational(4, -6)  #=> (-2/3)
# 3.to_r           #=> (3/1)
# 2/3r             #=> (2/3)
# ```
#
# You can also create rational objects from floating-point numbers or strings.
#
# ```ruby
# Rational(0.3)    #=> (5404319552844595/18014398509481984)
# Rational('0.3')  #=> (3/10)
# Rational('2/3')  #=> (2/3)
#
# 0.3.to_r         #=> (5404319552844595/18014398509481984)
# '0.3'.to_r       #=> (3/10)
# '2/3'.to_r       #=> (2/3)
# 0.3.rationalize  #=> (3/10)
# ```
#
# A rational object is an exact number, which helps you to write programs
# without any rounding errors.
#
# ```ruby
# 10.times.inject(0) {|t| t + 0.1 }              #=> 0.9999999999999999
# 10.times.inject(0) {|t| t + Rational('0.1') }  #=> (1/1)
# ```
#
# However, when an expression includes an inexact component (numerical value or
# operation), it will produce an inexact result.
#
# ```ruby
# Rational(10) / 3   #=> (10/3)
# Rational(10) / 3.0 #=> 3.3333333333333335
#
# Rational(-8) ** Rational(1, 3)
#                    #=> (1.0000000000000002+1.7320508075688772i)
# ```
class Rational < ::Numeric
  # Returns the value as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # The required `precision` parameter is used to determine the number of
  # significant digits for the result.
  #
  # ```ruby
  # require 'bigdecimal'
  # require 'bigdecimal/util'
  #
  # Rational(22, 7).to_d(3)   # => 0.314e1
  # ```
  #
  # See also
  # [`BigDecimal::new`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-c-new).
  sig { params(precision: Integer).returns(BigDecimal) }
  def to_d(precision); end
end

# A [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object holds and
# manipulates an arbitrary sequence of bytes, typically representing characters.
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) objects may be
# created using
# [`String::new`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-c-new)
# or as literals.
#
# Because of aliasing issues, users of strings should be aware of the methods
# that modify the contents of a
# [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object. Typically,
# methods with names ending in "!" modify their receiver, while those without a
# "!" return a new [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
# However, there are exceptions, such as
# [`String#[]=`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-5B-5D-3D).
class String
  # Returns the result of interpreting leading characters in `str` as a
  # [`BigDecimal`](https://docs.ruby-lang.org/en/2.7.0/BigDecimal.html).
  #
  # ```ruby
  # require 'bigdecimal'
  # require 'bigdecimal/util'
  #
  # "0.5".to_d             # => 0.5e0
  # "123.45e1".to_d        # => 0.12345e4
  # "45.67 degrees".to_d   # => 0.4567e2
  # ```
  #
  # See also
  # [`BigDecimal::new`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-c-new).
  sig { returns(BigDecimal) }
  def to_d; end
end

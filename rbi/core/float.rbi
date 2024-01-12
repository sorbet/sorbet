# typed: __STDLIB_INTERNAL

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
class Float < Numeric
  # The minimum number of significant decimal digits in a double-precision
  # floating point.
  #
  # Usually defaults to 15.
  DIG = T.let(T.unsafe(nil), Integer)
  # The difference between 1 and the smallest double-precision floating point
  # number greater than 1.
  #
  # Usually defaults to 2.2204460492503131e-16.
  EPSILON = T.let(T.unsafe(nil), Float)
  # An expression representing positive infinity.
  INFINITY = T.let(T.unsafe(nil), Float)
  # The number of base digits for the `double` data type.
  #
  # Usually defaults to 53.
  MANT_DIG = T.let(T.unsafe(nil), Integer)
  # The largest possible integer in a double-precision floating point number.
  #
  # Usually defaults to 1.7976931348623157e+308.
  MAX = T.let(T.unsafe(nil), Float)
  # The largest positive exponent in a double-precision floating point where 10
  # raised to this power minus 1.
  #
  # Usually defaults to 308.
  MAX_10_EXP = T.let(T.unsafe(nil), Integer)
  # The largest possible exponent value in a double-precision floating point.
  #
  # Usually defaults to 1024.
  MAX_EXP = T.let(T.unsafe(nil), Integer)
  # The smallest positive normalized number in a double-precision floating
  # point.
  #
  # Usually defaults to 2.2250738585072014e-308.
  #
  # If the platform supports denormalized numbers, there are numbers between
  # zero and [`Float::MIN`](https://docs.ruby-lang.org/en/2.7.0/Float.html#MIN).
  # 0.0.next\_float returns the smallest positive floating point number
  # including denormalized numbers.
  MIN = T.let(T.unsafe(nil), Float)
  # The smallest negative exponent in a double-precision floating point where 10
  # raised to this power minus 1.
  #
  # Usually defaults to -307.
  MIN_10_EXP = T.let(T.unsafe(nil), Integer)
  # The smallest possible exponent value in a double-precision floating point.
  #
  # Usually defaults to -1021.
  MIN_EXP = T.let(T.unsafe(nil), Integer)
  # An expression representing a value which is "not a number".
  NAN = T.let(T.unsafe(nil), Float)
  # The base of the floating point, or number of unique digits used to represent
  # the number.
  #
  # Usually defaults to 2 on most systems, which would represent a base-10
  # decimal.
  RADIX = T.let(T.unsafe(nil), Integer)
  # Deprecated, do not use.
  #
  # Represents the rounding mode for floating point addition at the start time.
  #
  # Usually defaults to 1, rounding to the nearest number.
  #
  # Other modes include:
  #
  # -1
  # :   Indeterminable
  # 0
  # :   Rounding towards zero
  # 1
  # :   Rounding to the nearest number
  # 2
  # :   Rounding towards positive infinity
  # 3
  # :   Rounding towards negative infinity
  ROUNDS = T.let(T.unsafe(nil), Integer)

  # Returns the modulo after division of `float` by `other`.
  #
  # ```ruby
  # 6543.21.modulo(137)      #=> 104.21000000000004
  # 6543.21.modulo(137.24)   #=> 92.92999999999961
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def %(arg0); end

  # Returns a new [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html)
  # which is the product of `float` and `other`.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def *(arg0); end

  # Raises `float` to the power of `other`.
  #
  # ```ruby
  # 2.0**3   #=> 8.0
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
  def **(arg0); end

  # Returns a new [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html)
  # which is the sum of `float` and `other`.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def +(arg0); end

  sig {returns(Float)}
  def +@(); end

  # Returns a new [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html)
  # which is the difference of `float` and `other`.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def -(arg0); end

  # Returns `float`, negated.
  sig {returns(Float)}
  def -@(); end

  # Returns a new [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html)
  # which is the result of dividing `float` by `other`.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Complex,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def /(arg0); end

  # Returns `true` if `float` is less than `real`.
  #
  # The result of `NaN < NaN` is undefined, so an implementation-dependent value
  # is returned.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  # Returns `true` if `float` is less than or equal to `real`.
  #
  # The result of `NaN <= NaN` is undefined, so an implementation-dependent
  # value is returned.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  # Returns -1, 0, or +1 depending on whether `float` is less than, equal to, or
  # greater than `real`. This is the basis for the tests in the
  # [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html) module.
  #
  # The result of `NaN <=> NaN` is undefined, so an implementation-dependent
  # value is returned.
  #
  # `nil` is returned if the two values are incomparable.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Integer)
  end
  def <=>(arg0); end

  # Returns `true` only if `obj` has the same value as `float`. Contrast this
  # with
  # [`Float#eql?`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-eql-3F),
  # which requires `obj` to be a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html).
  #
  # ```ruby
  # 1.0 == 1   #=> true
  # ```
  #
  # The result of `NaN == NaN` is undefined, so an implementation-dependent
  # value is returned.
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns `true` only if `obj` has the same value as `float`. Contrast this
  # with
  # [`Float#eql?`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-eql-3F),
  # which requires `obj` to be a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html).
  #
  # ```ruby
  # 1.0 == 1   #=> true
  # ```
  #
  # The result of `NaN == NaN` is undefined, so an implementation-dependent
  # value is returned.
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  # Returns `true` if `float` is greater than `real`.
  #
  # The result of `NaN > NaN` is undefined, so an implementation-dependent value
  # is returned.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  # Returns `true` if `float` is greater than or equal to `real`.
  #
  # The result of `NaN >= NaN` is undefined, so an implementation-dependent
  # value is returned.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  # Returns the absolute value of `float`.
  #
  # ```ruby
  # (-34.56).abs   #=> 34.56
  # -34.56.abs     #=> 34.56
  # 34.56.abs      #=> 34.56
  # ```
  #
  # [`Float#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-magnitude)
  # is an alias for
  # [`Float#abs`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-abs).
  sig {returns(Float)}
  def abs(); end

  sig {returns(Float)}
  def abs2(); end

  # Returns 0 if the value is positive, pi otherwise.
  sig {returns(T.any(Integer, Float))}
  def angle(); end

  # Returns 0 if the value is positive, pi otherwise.
  sig {returns(T.any(Integer, Float))}
  def arg(); end

  # Returns the smallest number greater than or equal to `float` with a
  # precision of `ndigits` decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a floating point number when `ndigits` is positive, otherwise
  # returns an integer.
  #
  # ```ruby
  # 1.2.ceil      #=> 2
  # 2.0.ceil      #=> 2
  # (-1.2).ceil   #=> -1
  # (-2.0).ceil   #=> -2
  #
  # 1.234567.ceil(2)   #=> 1.24
  # 1.234567.ceil(3)   #=> 1.235
  # 1.234567.ceil(4)   #=> 1.2346
  # 1.234567.ceil(5)   #=> 1.23457
  #
  # 34567.89.ceil(-5)  #=> 100000
  # 34567.89.ceil(-4)  #=> 40000
  # 34567.89.ceil(-3)  #=> 35000
  # 34567.89.ceil(-2)  #=> 34600
  # 34567.89.ceil(-1)  #=> 34570
  # 34567.89.ceil(0)   #=> 34568
  # 34567.89.ceil(1)   #=> 34567.9
  # 34567.89.ceil(2)   #=> 34567.89
  # 34567.89.ceil(3)   #=> 34567.89
  # ```
  #
  # Note that the limited precision of floating point arithmetic might lead to
  # surprising results:
  #
  # ```ruby
  # (2.1 / 0.7).ceil  #=> 4 (!)
  # ```
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  # Returns an array with both `numeric` and `float` represented as
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) objects.
  #
  # This is achieved by converting `numeric` to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html).
  #
  # ```ruby
  # 1.2.coerce(3)       #=> [3.0, 1.2]
  # 2.5.coerce(1.1)     #=> [1.1, 2.5]
  # ```
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([Float, Float])
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Float, Float])
  end
  def coerce(arg0); end

  sig {returns(Float)}
  def conj(); end

  sig {returns(Float)}
  def conjugate(); end

  # Returns the denominator (always positive). The result is machine dependent.
  #
  # See also
  # [`Float#numerator`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-numerator).
  sig {returns(Integer)}
  def denominator(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Integer)
  end
  def div(arg0); end

  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod).
  #
  # ```ruby
  # 42.0.divmod(6)   #=> [7, 0.0]
  # 42.0.divmod(5)   #=> [8, 2.0]
  # ```
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def divmod(arg0); end

  # Returns `true` only if `obj` is a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) with the same
  # value as `float`. Contrast this with Float#==, which performs type
  # conversions.
  #
  # ```ruby
  # 1.0.eql?(1)   #=> false
  # ```
  #
  # The result of `NaN.eql?(NaN)` is undefined, so an implementation-dependent
  # value is returned.
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

  # Returns `float / numeric`, same as Float#/.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Complex,
    )
    .returns(Complex)
  end
  def fdiv(arg0); end

  # Returns `true` if `float` is a valid IEEE floating point number, i.e. it is
  # not infinite and
  # [`Float#nan?`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-nan-3F)
  # is `false`.
  sig {returns(T::Boolean)}
  def finite?(); end

  # Returns the largest number less than or equal to `float` with a precision of
  # `ndigits` decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a floating point number when `ndigits` is positive, otherwise
  # returns an integer.
  #
  # ```ruby
  # 1.2.floor      #=> 1
  # 2.0.floor      #=> 2
  # (-1.2).floor   #=> -2
  # (-2.0).floor   #=> -2
  #
  # 1.234567.floor(2)   #=> 1.23
  # 1.234567.floor(3)   #=> 1.234
  # 1.234567.floor(4)   #=> 1.2345
  # 1.234567.floor(5)   #=> 1.23456
  #
  # 34567.89.floor(-5)  #=> 0
  # 34567.89.floor(-4)  #=> 30000
  # 34567.89.floor(-3)  #=> 34000
  # 34567.89.floor(-2)  #=> 34500
  # 34567.89.floor(-1)  #=> 34560
  # 34567.89.floor(0)   #=> 34567
  # 34567.89.floor(1)   #=> 34567.8
  # 34567.89.floor(2)   #=> 34567.89
  # 34567.89.floor(3)   #=> 34567.89
  # ```
  #
  # Note that the limited precision of floating point arithmetic might lead to
  # surprising results:
  #
  # ```ruby
  # (0.3 / 0.1).floor  #=> 2 (!)
  # ```
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

  # Returns a hash code for this float.
  #
  # See also
  # [`Object#hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash).
  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  # Returns `nil`, -1, or 1 depending on whether the value is finite,
  # `-Infinity`, or `+Infinity`.
  #
  # ```ruby
  # (0.0).infinite?        #=> nil
  # (-1.0/0.0).infinite?   #=> -1
  # (+1.0/0.0).infinite?   #=> 1
  # ```
  sig {returns(T.nilable(Integer))}
  def infinite?(); end

  # Alias for:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

  # Returns the absolute value of `float`.
  #
  # ```ruby
  # (-34.56).abs   #=> 34.56
  # -34.56.abs     #=> 34.56
  # 34.56.abs      #=> 34.56
  # ```
  #
  # [`Float#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-magnitude)
  # is an alias for
  # [`Float#abs`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-abs).
  sig {returns(Float)}
  def magnitude(); end

  # Returns the modulo after division of `float` by `other`.
  #
  # ```ruby
  # 6543.21.modulo(137)      #=> 104.21000000000004
  # 6543.21.modulo(137.24)   #=> 92.92999999999961
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def modulo(arg0); end

  # Returns `true` if `float` is an invalid IEEE floating point number.
  #
  # ```ruby
  # a = -1.0      #=> -1.0
  # a.nan?        #=> false
  # a = 0.0/0.0   #=> NaN
  # a.nan?        #=> true
  # ```
  sig {returns(T::Boolean)}
  def nan?(); end

  # Returns `true` if `float` is less than 0.
  def negative?; end

  # Returns the next representable floating point number.
  #
  # Float::MAX.next\_float and Float::INFINITY.next\_float is
  # [`Float::INFINITY`](https://docs.ruby-lang.org/en/2.7.0/Float.html#INFINITY).
  #
  # Float::NAN.next\_float is
  # [`Float::NAN`](https://docs.ruby-lang.org/en/2.7.0/Float.html#NAN).
  #
  # For example:
  #
  # ```ruby
  # 0.01.next_float    #=> 0.010000000000000002
  # 1.0.next_float     #=> 1.0000000000000002
  # 100.0.next_float   #=> 100.00000000000001
  #
  # 0.01.next_float - 0.01     #=> 1.734723475976807e-18
  # 1.0.next_float - 1.0       #=> 2.220446049250313e-16
  # 100.0.next_float - 100.0   #=> 1.4210854715202004e-14
  #
  # f = 0.01; 20.times { printf "%-20a %s\n", f, f.to_s; f = f.next_float }
  # #=> 0x1.47ae147ae147bp-7 0.01
  # #   0x1.47ae147ae147cp-7 0.010000000000000002
  # #   0x1.47ae147ae147dp-7 0.010000000000000004
  # #   0x1.47ae147ae147ep-7 0.010000000000000005
  # #   0x1.47ae147ae147fp-7 0.010000000000000007
  # #   0x1.47ae147ae148p-7  0.010000000000000009
  # #   0x1.47ae147ae1481p-7 0.01000000000000001
  # #   0x1.47ae147ae1482p-7 0.010000000000000012
  # #   0x1.47ae147ae1483p-7 0.010000000000000014
  # #   0x1.47ae147ae1484p-7 0.010000000000000016
  # #   0x1.47ae147ae1485p-7 0.010000000000000018
  # #   0x1.47ae147ae1486p-7 0.01000000000000002
  # #   0x1.47ae147ae1487p-7 0.010000000000000021
  # #   0x1.47ae147ae1488p-7 0.010000000000000023
  # #   0x1.47ae147ae1489p-7 0.010000000000000024
  # #   0x1.47ae147ae148ap-7 0.010000000000000026
  # #   0x1.47ae147ae148bp-7 0.010000000000000028
  # #   0x1.47ae147ae148cp-7 0.01000000000000003
  # #   0x1.47ae147ae148dp-7 0.010000000000000031
  # #   0x1.47ae147ae148ep-7 0.010000000000000033
  #
  # f = 0.0
  # 100.times { f += 0.1 }
  # f                           #=> 9.99999999999998       # should be 10.0 in the ideal world.
  # 10-f                        #=> 1.9539925233402755e-14 # the floating point error.
  # 10.0.next_float-10          #=> 1.7763568394002505e-15 # 1 ulp (unit in the last place).
  # (10-f)/(10.0.next_float-10) #=> 11.0                   # the error is 11 ulp.
  # (10-f)/(10*Float::EPSILON)  #=> 8.8                    # approximation of the above.
  # "%a" % 10                   #=> "0x1.4p+3"
  # "%a" % f                    #=> "0x1.3fffffffffff5p+3" # the last hex digit is 5.  16 - 5 = 11 ulp.
  # ```
  sig {returns(Float)}
  def next_float(); end

  # Returns the numerator. The result is machine dependent.
  #
  # ```ruby
  # n = 0.3.numerator    #=> 5404319552844595
  # d = 0.3.denominator  #=> 18014398509481984
  # n.fdiv(d)            #=> 0.3
  # ```
  #
  # See also
  # [`Float#denominator`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-denominator).
  sig {returns(Integer)}
  def numerator(); end

  # Returns 0 if the value is positive, pi otherwise.
  sig {returns(T.any(Integer, Float))}
  def phase(); end

  # Returns `true` if `float` is greater than 0.
  def positive?; end

  # Returns the previous representable floating point number.
  #
  # (-Float::MAX).prev\_float and (-Float::INFINITY).prev\_float is
  # -Float::INFINITY.
  #
  # Float::NAN.prev\_float is
  # [`Float::NAN`](https://docs.ruby-lang.org/en/2.7.0/Float.html#NAN).
  #
  # For example:
  #
  # ```ruby
  # 0.01.prev_float    #=> 0.009999999999999998
  # 1.0.prev_float     #=> 0.9999999999999999
  # 100.0.prev_float   #=> 99.99999999999999
  #
  # 0.01 - 0.01.prev_float     #=> 1.734723475976807e-18
  # 1.0 - 1.0.prev_float       #=> 1.1102230246251565e-16
  # 100.0 - 100.0.prev_float   #=> 1.4210854715202004e-14
  #
  # f = 0.01; 20.times { printf "%-20a %s\n", f, f.to_s; f = f.prev_float }
  # #=> 0x1.47ae147ae147bp-7 0.01
  # #   0x1.47ae147ae147ap-7 0.009999999999999998
  # #   0x1.47ae147ae1479p-7 0.009999999999999997
  # #   0x1.47ae147ae1478p-7 0.009999999999999995
  # #   0x1.47ae147ae1477p-7 0.009999999999999993
  # #   0x1.47ae147ae1476p-7 0.009999999999999992
  # #   0x1.47ae147ae1475p-7 0.00999999999999999
  # #   0x1.47ae147ae1474p-7 0.009999999999999988
  # #   0x1.47ae147ae1473p-7 0.009999999999999986
  # #   0x1.47ae147ae1472p-7 0.009999999999999985
  # #   0x1.47ae147ae1471p-7 0.009999999999999983
  # #   0x1.47ae147ae147p-7  0.009999999999999981
  # #   0x1.47ae147ae146fp-7 0.00999999999999998
  # #   0x1.47ae147ae146ep-7 0.009999999999999978
  # #   0x1.47ae147ae146dp-7 0.009999999999999976
  # #   0x1.47ae147ae146cp-7 0.009999999999999974
  # #   0x1.47ae147ae146bp-7 0.009999999999999972
  # #   0x1.47ae147ae146ap-7 0.00999999999999997
  # #   0x1.47ae147ae1469p-7 0.009999999999999969
  # #   0x1.47ae147ae1468p-7 0.009999999999999967
  # ```
  sig {returns(Float)}
  def prev_float(); end

  # Returns `float / numeric`, same as Float#/.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
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
        arg0: Complex,
    )
    .returns(Complex)
  end
  def quo(arg0); end

  # Returns a simpler approximation of the value (flt-|eps| <= result <=
  # flt+|eps|). If the optional argument `eps` is not given, it will be chosen
  # automatically.
  #
  # ```ruby
  # 0.3.rationalize          #=> (3/10)
  # 1.333.rationalize        #=> (1333/1000)
  # 1.333.rationalize(0.01)  #=> (4/3)
  # ```
  #
  # See also
  # [`Float#to_r`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-to_r).
  sig {returns(Rational)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Rational)
  end
  def rationalize(arg0=T.unsafe(nil)); end

  sig {returns(Float)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  # Returns `float` rounded to the nearest value with a precision of `ndigits`
  # decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a floating point number when `ndigits` is positive, otherwise
  # returns an integer.
  #
  # ```ruby
  # 1.4.round      #=> 1
  # 1.5.round      #=> 2
  # 1.6.round      #=> 2
  # (-1.5).round   #=> -2
  #
  # 1.234567.round(2)   #=> 1.23
  # 1.234567.round(3)   #=> 1.235
  # 1.234567.round(4)   #=> 1.2346
  # 1.234567.round(5)   #=> 1.23457
  #
  # 34567.89.round(-5)  #=> 0
  # 34567.89.round(-4)  #=> 30000
  # 34567.89.round(-3)  #=> 35000
  # 34567.89.round(-2)  #=> 34600
  # 34567.89.round(-1)  #=> 34570
  # 34567.89.round(0)   #=> 34568
  # 34567.89.round(1)   #=> 34567.9
  # 34567.89.round(2)   #=> 34567.89
  # 34567.89.round(3)   #=> 34567.89
  # ```
  #
  # If the optional `half` keyword argument is given, numbers that are half-way
  # between two possible rounded values will be rounded according to the
  # specified tie-breaking `mode`:
  #
  # *   `:up` or `nil`: round half away from zero (default)
  # *   `:down`: round half toward zero
  # *   `:even`: round half toward the nearest even number
  #
  # ```ruby
  # 2.5.round(half: :up)      #=> 3
  # 2.5.round(half: :down)    #=> 2
  # 2.5.round(half: :even)    #=> 2
  # 3.5.round(half: :up)      #=> 4
  # 3.5.round(half: :down)    #=> 3
  # 3.5.round(half: :even)    #=> 4
  # (-2.5).round(half: :up)   #=> -3
  # (-2.5).round(half: :down) #=> -2
  # (-2.5).round(half: :even) #=> -2
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float))
  end
  def round(arg0=T.unsafe(nil)); end

  sig {returns(Complex)}
  def to_c(); end

  # Since `float` is already a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html), returns `self`.
  sig {returns(Float)}
  def to_f(); end

  # Returns the `float` truncated to an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # ```ruby
  # 1.2.to_i      #=> 1
  # (-1.2).to_i   #=> -1
  # ```
  #
  # Note that the limited precision of floating point arithmetic might lead to
  # surprising results:
  #
  # ```ruby
  # (0.3 / 0.1).to_i  #=> 2 (!)
  # ```
  #
  # [`to_int`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-to_int)
  # is an alias for
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-to_i).
  sig {returns(Integer)}
  def to_i(); end

  # Returns the `float` truncated to an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # ```ruby
  # 1.2.to_i      #=> 1
  # (-1.2).to_i   #=> -1
  # ```
  #
  # Note that the limited precision of floating point arithmetic might lead to
  # surprising results:
  #
  # ```ruby
  # (0.3 / 0.1).to_i  #=> 2 (!)
  # ```
  #
  # [`to_int`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-to_int)
  # is an alias for
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-to_i).
  sig {returns(Integer)}
  def to_int(); end

  # Returns the value as a rational.
  #
  # ```ruby
  # 2.0.to_r    #=> (2/1)
  # 2.5.to_r    #=> (5/2)
  # -0.75.to_r  #=> (-3/4)
  # 0.0.to_r    #=> (0/1)
  # 0.3.to_r    #=> (5404319552844595/18014398509481984)
  # ```
  #
  # NOTE: 0.3.to\_r isn't the same as "0.3".to\_r. The latter is equivalent to
  # "3/10".to\_r, but the former isn't so.
  #
  # ```ruby
  # 0.3.to_r   == 3/10r  #=> false
  # "0.3".to_r == 3/10r  #=> true
  # ```
  #
  # See also
  # [`Float#rationalize`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-rationalize).
  sig {returns(Rational)}
  def to_r(); end

  # Returns a string containing a representation of `self`. As well as a fixed
  # or exponential form of the `float`, the call may return `NaN`, `Infinity`,
  # and `-Infinity`.
  #
  # Also aliased as:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-inspect)
  sig {returns(String)}
  def to_s(); end

  # Returns `float` truncated (toward zero) to a precision of `ndigits` decimal
  # digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a floating point number when `ndigits` is positive, otherwise
  # returns an integer.
  #
  # ```ruby
  # 2.8.truncate           #=> 2
  # (-2.8).truncate        #=> -2
  # 1.234567.truncate(2)   #=> 1.23
  # 34567.89.truncate(-2)  #=> 34500
  # ```
  #
  # Note that the limited precision of floating point arithmetic might lead to
  # surprising results:
  #
  # ```ruby
  # (0.3 / 0.1).truncate  #=> 2 (!)
  # ```
  sig {returns(Integer)}
  sig do
    params(
      arg0: Integer
    )
    .returns(T.any(Integer, Float))
  end
  def truncate(arg0=T.unsafe(nil)); end

  # Returns `true` if `float` is 0.0.
  sig {returns(T::Boolean)}
  def zero?(); end
end

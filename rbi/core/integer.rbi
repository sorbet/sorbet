# typed: __STDLIB_INTERNAL

# Holds [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) values.
# You cannot add a singleton method to an
# [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) object, any
# attempt to do so will raise a
# [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html).
class Integer < Numeric
  # Returns `int` modulo `other`.
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod)
  # for more information.
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.any(Integer, Float))
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def %(arg0); end

  # Bitwise AND.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def &(arg0); end

  # Performs multiplication: the class of the resulting object depends on the
  # class of `numeric`.
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
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
    .returns(T.any(Integer, Float))
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def *(arg0); end

  # Raises `int` to the power of `numeric`, which may be negative or fractional.
  # The result may be an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html), a
  # [`Rational`](https://docs.ruby-lang.org/en/2.7.0/Rational.html), or a
  # complex number.
  #
  # ```ruby
  # 2 ** 3        #=> 8
  # 2 ** -1       #=> (1/2)
  # 2 ** 0.5      #=> 1.4142135623730951
  # (-1) ** 0.5   #=> (0.0+1.0i)
  #
  # 123456789 ** 2     #=> 15241578750190521
  # 123456789 ** 1.2   #=> 5126464716.0993185
  # 123456789 ** -2    #=> (1/15241578750190521)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Numeric)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Numeric)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Numeric)
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

  # Performs addition: the class of the resulting object depends on the class of
  # `numeric`.
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
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
    .returns(T.any(Integer, Float))
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def +(arg0); end

  sig {returns(Integer)}
  def +@(); end

  # Performs subtraction: the class of the resulting object depends on the class
  # of `numeric`.
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
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
    .returns(T.any(Integer, Float))
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def -(arg0); end

  # Returns `int`, negated.
  sig {returns(Integer)}
  def -@(); end

  # Performs division: the class of the resulting object depends on the class of
  # `numeric`.
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
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
    .returns(T.any(Integer, Float))
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def /(arg0); end

  # Returns `true` if the value of `int` is less than that of `real`.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  # Returns `int` shifted left `count` positions, or right if `count` is
  # negative.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def <<(arg0); end

  # Returns `true` if the value of `int` is less than or equal to that of
  # `real`.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  # Comparison---Returns -1, 0, or +1 depending on whether `int` is less than,
  # equal to, or greater than `numeric`.
  #
  # This is the basis for the tests in the
  # [`Comparable`](https://docs.ruby-lang.org/en/2.7.0/Comparable.html) module.
  #
  # `nil` is returned if the two values are incomparable.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Integer)
  end
  def <=>(arg0); end

  # Returns `true` if `int` equals `other` numerically. Contrast this with
  # [`Integer#eql?`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-eql-3F),
  # which requires `other` to be an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # ```ruby
  # 1 == 2     #=> false
  # 1 == 1.0   #=> true
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns `true` if `int` equals `other` numerically. Contrast this with
  # [`Integer#eql?`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-eql-3F),
  # which requires `other` to be an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html).
  #
  # ```ruby
  # 1 == 2     #=> false
  # 1 == 1.0   #=> true
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  # Returns `true` if the value of `int` is greater than that of `real`.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  # Returns `true` if the value of `int` is greater than or equal to that of
  # `real`.
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  # Returns `int` shifted right `count` positions, or left if `count` is
  # negative.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def >>(arg0); end

  # Bit Reference---Returns the `n`th bit in the binary representation of `int`,
  # where `int[0]` is the least significant bit.
  #
  # ```ruby
  # a = 0b11001100101010
  # 30.downto(0) {|n| print a[n] }
  # #=> 0000000000000000011001100101010
  #
  # a = 9**15
  # 50.downto(0) {|n| print a[n] }
  # #=> 000101110110100000111000011110010100111100010111001
  # ```
  #
  # In principle, `n[i]` is equivalent to `(n >> i) & 1`. Thus, any negative
  # index always returns zero:
  #
  # ```ruby
  # p 255[-1] #=> 0
  # ```
  #
  # [`Range`](https://docs.ruby-lang.org/en/2.7.0/Range.html) operations `n[i,
  # len]` and `n[i..j]` are naturally extended.
  #
  # *   `n[i, len]` equals to `(n >> i) & ((1 << len) - 1)`.
  # *   `n[i..j]` equals to `(n >> i) & ((1 << (j - i + 1)) - 1)`.
  # *   `n[i...j]` equals to `(n >> i) & ((1 << (j - i)) - 1)`.
  # *   `n[i..]` equals to `(n >> i)`.
  # *   `n[..j]` is zero if `n & ((1 << (j + 1)) - 1)` is zero. Otherwise,
  #     raises an
  #     [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html).
  # *   `n[...j]` is zero if `n & ((1 << j) - 1)` is zero. Otherwise, raises an
  #     [`ArgumentError`](https://docs.ruby-lang.org/en/2.7.0/ArgumentError.html).
  #
  #
  # Note that range operation may exhaust memory. For example, `-1[0,
  # 1000000000000]` will raise
  # [`NoMemoryError`](https://docs.ruby-lang.org/en/2.7.0/NoMemoryError.html).
  sig do
    params(
        arg0: Integer,
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
        arg0: Float,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Integer)
  end
  sig do
    params(
      arg0: T::Range[Integer],
    )
    .returns(Integer)
  end
  def [](arg0); end

  # Bitwise EXCLUSIVE OR.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def ^(arg0); end

  # Returns the absolute value of `int`.
  #
  # ```ruby
  # (-12345).abs   #=> 12345
  # -12345.abs     #=> 12345
  # 12345.abs      #=> 12345
  # ```
  #
  # [`Integer#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-magnitude)
  # is an alias for
  # [`Integer#abs`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-abs).
  sig {returns(Integer)}
  def abs(); end

  sig {returns(Integer)}
  def abs2(); end

  # Returns `true` if all bits of `int & mask` are 1.
  def allbits?(_); end

  sig {returns(Numeric)}
  def angle(); end

  # Returns `true` if any bits of `int & mask` are 1.
  def anybits?(_); end

  sig {returns(Numeric)}
  def arg(); end

  # Returns the number of bits of the value of `int`.
  #
  # "Number of bits" means the bit position of the highest bit which is
  # different from the sign bit (where the least significant bit has bit
  # position 1). If there is no such bit (zero or minus one), zero is returned.
  #
  # I.e. this method returns *ceil(log2(int < 0 ? -int : int+1))*.
  #
  # ```ruby
  # (-2**1000-1).bit_length   #=> 1001
  # (-2**1000).bit_length     #=> 1000
  # (-2**1000+1).bit_length   #=> 1000
  # (-2**12-1).bit_length     #=> 13
  # (-2**12).bit_length       #=> 12
  # (-2**12+1).bit_length     #=> 12
  # -0x101.bit_length         #=> 9
  # -0x100.bit_length         #=> 8
  # -0xff.bit_length          #=> 8
  # -2.bit_length             #=> 1
  # -1.bit_length             #=> 0
  # 0.bit_length              #=> 0
  # 1.bit_length              #=> 1
  # 0xff.bit_length           #=> 8
  # 0x100.bit_length          #=> 9
  # (2**12-1).bit_length      #=> 12
  # (2**12).bit_length        #=> 13
  # (2**12+1).bit_length      #=> 13
  # (2**1000-1).bit_length    #=> 1000
  # (2**1000).bit_length      #=> 1001
  # (2**1000+1).bit_length    #=> 1001
  # ```
  #
  # This method can be used to detect overflow in
  # [`Array#pack`](https://docs.ruby-lang.org/en/2.7.0/Array.html#method-i-pack)
  # as follows:
  #
  # ```ruby
  # if n.bit_length < 32
  #   [n].pack("l") # no overflow
  # else
  #   raise "overflow"
  # end
  # ```
  sig {returns(Integer)}
  def bit_length(); end

  # Returns the smallest number greater than or equal to `int` with a precision
  # of `ndigits` decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns `self` when `ndigits` is zero or positive.
  #
  # ```ruby
  # 1.ceil           #=> 1
  # 1.ceil(2)        #=> 1
  # 18.ceil(-1)      #=> 20
  # (-18).ceil(-1)   #=> -10
  # ```
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  # Returns a string containing the character represented by the `int`'s value
  # according to `encoding`.
  #
  # ```ruby
  # 65.chr    #=> "A"
  # 230.chr   #=> "\xE6"
  # 255.chr(Encoding::UTF_8)   #=> "\u00FF"
  # ```
  sig {returns(String)}
  sig do
    params(
        arg0: T.any(Encoding, String),
    )
    .returns(String)
  end
  def chr(arg0=T.unsafe(nil)); end

  # Returns an array with both a `numeric` and a `big` represented as Bignum
  # objects.
  #
  # This is achieved by converting `numeric` to a Bignum.
  #
  # A [`TypeError`](https://docs.ruby-lang.org/en/2.7.0/TypeError.html) is
  # raised if the `numeric` is not a Fixnum or Bignum type.
  #
  # ```ruby
  # (0x3FFFFFFFFFFFFFFF+1).coerce(42)   #=> [42, 4611686018427387904]
  # ```
  sig do
    params(
        arg0: Numeric,
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def coerce(arg0); end

  sig {returns(Integer)}
  def conj(); end

  sig {returns(Integer)}
  def conjugate(); end

  # Returns 1.
  sig {returns(Integer)}
  def denominator(); end

  # Returns the digits of `int`'s place-value representation with radix `base`
  # (default: 10). The digits are returned as an array with the least
  # significant digit as the first array element.
  #
  # `base` must be greater than or equal to 2.
  #
  # ```ruby
  # 12345.digits      #=> [5, 4, 3, 2, 1]
  # 12345.digits(7)   #=> [4, 6, 6, 0, 5]
  # 12345.digits(100) #=> [45, 23, 1]
  #
  # -12345.digits(7)  #=> Math::DomainError
  # ```
  def digits(*_); end

  # Performs integer division: returns the integer result of dividing `int` by
  # `numeric`.
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
  sig do
    params(
        arg0: Integer,
    )
    .returns([Integer, Integer])
  end
  sig do
    params(
        arg0: T.any(Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def divmod(arg0); end

  # Iterates the given block, passing in decreasing values from `int` down to
  # and including `limit`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # 5.downto(1) { |n| print n, ".. " }
  # puts "Liftoff!"
  # #=> "5.. 4.. 3.. 2.. 1.. Liftoff!"
  # ```
  sig do
    params(
        limit: Integer,
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig do
    params(
        limit: Integer,
    )
    .returns(T::Enumerator[Integer])
  end
  def downto(limit, &blk); end

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

  # Returns `true` if `int` is an even number.
  sig {returns(T::Boolean)}
  def even?(); end

  # Returns the floating point result of dividing `int` by `numeric`.
  #
  # ```ruby
  # 654321.fdiv(13731)      #=> 47.652829364212366
  # 654321.fdiv(13731.24)   #=> 47.65199646936475
  # -654321.fdiv(13731)     #=> -47.652829364212366
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
  def fdiv(arg0); end

  # Returns the largest number less than or equal to `int` with a precision of
  # `ndigits` decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns `self` when `ndigits` is zero or positive.
  #
  # ```ruby
  # 1.floor           #=> 1
  # 1.floor(2)        #=> 1
  # 18.floor(-1)      #=> 10
  # (-18).floor(-1)   #=> -20
  # ```
  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

  # Returns the greatest common divisor of the two integers. The result is
  # always positive. 0.gcd(x) and x.gcd(0) return x.abs.
  #
  # ```ruby
  # 36.gcd(60)                  #=> 12
  # 2.gcd(2)                    #=> 2
  # 3.gcd(-7)                   #=> 1
  # ((1<<31)-1).gcd((1<<61)-1)  #=> 1
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def gcd(arg0); end

  # Returns an array with the greatest common divisor and the least common
  # multiple of the two integers, [gcd, lcm].
  #
  # ```ruby
  # 36.gcdlcm(60)                  #=> [12, 180]
  # 2.gcdlcm(2)                    #=> [2, 2]
  # 3.gcdlcm(-7)                   #=> [1, 21]
  # ((1<<31)-1).gcdlcm((1<<61)-1)  #=> [1, 4951760154835678088235319297]
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns([Integer, Integer])
  end
  def gcdlcm(arg0); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  # Alias for:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

  # Since `int` is already an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), this always
  # returns `true`.
  sig {returns(TrueClass)}
  def integer?(); end

  # Returns the least common multiple of the two integers. The result is always
  # positive. 0.lcm(x) and x.lcm(0) return zero.
  #
  # ```ruby
  # 36.lcm(60)                  #=> 180
  # 2.lcm(2)                    #=> 2
  # 3.lcm(-7)                   #=> 21
  # ((1<<31)-1).lcm((1<<61)-1)  #=> 4951760154835678088235319297
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def lcm(arg0); end

  # Returns the absolute value of `int`.
  #
  # ```ruby
  # (-12345).abs   #=> 12345
  # -12345.abs     #=> 12345
  # 12345.abs      #=> 12345
  # ```
  #
  # [`Integer#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-magnitude)
  # is an alias for
  # [`Integer#abs`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-abs).
  sig {returns(Integer)}
  def magnitude(); end

  # Returns `int` modulo `other`.
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod)
  # for more information.
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def modulo(arg0); end

  # Returns the successor of `int`, i.e. the
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) equal to
  # `int+1`.
  #
  # ```ruby
  # 1.next      #=> 2
  # (-1).next   #=> 0
  # 1.succ      #=> 2
  # (-1).succ   #=> 0
  # ```
  sig {returns(Integer)}
  def next(); end

  # Returns `true` if no bits of `int & mask` are 1.
  def nobits?(_); end

  # Returns self.
  sig {returns(Integer)}
  def numerator(); end

  # Returns `true` if `int` is an odd number.
  sig {returns(T::Boolean)}
  def odd?(); end

  # Returns the `int` itself.
  #
  # ```ruby
  # 97.ord   #=> 97
  # ```
  #
  # This method is intended for compatibility to character literals in Ruby 1.9.
  #
  # For example, `?a.ord` returns 97 both in 1.8 and 1.9.
  sig {returns(Integer)}
  def ord(); end

  sig {returns(Numeric)}
  def phase(); end

  # Returns (modular) exponentiation as:
  #
  # ```ruby
  # a.pow(b)     #=> same as a**b
  # a.pow(b, m)  #=> same as (a**b) % m, but avoids huge temporary values
  # ```
  def pow(*_); end

  # Returns the predecessor of `int`, i.e. the
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) equal to
  # `int-1`.
  #
  # ```ruby
  # 1.pred      #=> 0
  # (-1).pred   #=> -2
  # ```
  sig {returns(Integer)}
  def pred(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
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
    .returns(Rational)
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

  # Returns the value as a rational. The optional argument `eps` is always
  # ignored.
  sig {returns(Rational)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Rational)
  end
  def rationalize(arg0=T.unsafe(nil)); end

  sig {returns(Integer)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  # Returns the remainder after dividing `int` by `numeric`.
  #
  # `x.remainder(y)` means `x-y*(x/y).truncate`.
  #
  # ```ruby
  # 5.remainder(3)     #=> 2
  # -5.remainder(3)    #=> -2
  # 5.remainder(-3)    #=> 2
  # -5.remainder(-3)   #=> -2
  # 5.remainder(1.5)   #=> 0.5
  # ```
  #
  # See
  # [`Numeric#divmod`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html#method-i-divmod).
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def remainder(arg0); end

  # Returns `int` rounded to the nearest value with a precision of `ndigits`
  # decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns `self` when `ndigits` is zero or positive.
  #
  # ```ruby
  # 1.round           #=> 1
  # 1.round(2)        #=> 1
  # 15.round(-1)      #=> 20
  # (-15).round(-1)   #=> -20
  # ```
  #
  # The optional `half` keyword argument is available similar to
  # [`Float#round`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-round).
  #
  # ```ruby
  # 25.round(-1, half: :up)      #=> 30
  # 25.round(-1, half: :down)    #=> 20
  # 25.round(-1, half: :even)    #=> 20
  # 35.round(-1, half: :up)      #=> 40
  # 35.round(-1, half: :down)    #=> 30
  # 35.round(-1, half: :even)    #=> 40
  # (-25).round(-1, half: :up)   #=> -30
  # (-25).round(-1, half: :down) #=> -20
  # (-25).round(-1, half: :even) #=> -20
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Integer)
  end
  def round(arg0=T.unsafe(nil)); end

  # Returns the number of bytes in the machine representation of `int` (machine
  # dependent).
  #
  # ```ruby
  # 1.size               #=> 8
  # -1.size              #=> 8
  # 2147483647.size      #=> 8
  # (256**10 - 1).size   #=> 10
  # (256**20 - 1).size   #=> 20
  # (256**40 - 1).size   #=> 40
  # ```
  sig {returns(Integer)}
  def size(); end

  # Returns the successor of `int`, i.e. the
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) equal to
  # `int+1`.
  #
  # ```ruby
  # 1.next      #=> 2
  # (-1).next   #=> 0
  # 1.succ      #=> 2
  # (-1).succ   #=> 0
  # ```
  sig {returns(Integer)}
  def succ(); end

  # Iterates the given block `int` times, passing in values from zero to `int -
  # 1`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # 5.times {|i| print i, " " }   #=> 0 1 2 3 4
  # ```
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig {returns(T::Enumerator[Integer])}
  def times(&blk); end

  sig {returns(Complex)}
  def to_c(); end

  # Converts `int` to a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html). If `int` doesn't
  # fit in a [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html), the
  # result is infinity.
  sig {returns(Float)}
  def to_f(); end

  # Since `int` is already an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), returns
  # `self`.
  #
  # [`to_int`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-to_int)
  # is an alias for
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-to_i).
  sig {returns(Integer)}
  def to_i(); end

  # Since `int` is already an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), returns
  # `self`.
  #
  # [`to_int`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-to_int)
  # is an alias for
  # [`to_i`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-to_i).
  sig {returns(Integer)}
  def to_int(); end

  # Returns the value as a rational.
  #
  # ```ruby
  # 1.to_r        #=> (1/1)
  # (1<<64).to_r  #=> (18446744073709551616/1)
  # ```
  sig {returns(Rational)}
  def to_r(); end

  # Returns a string containing the place-value representation of `int` with
  # radix `base` (between 2 and 36).
  #
  # ```ruby
  # 12345.to_s       #=> "12345"
  # 12345.to_s(2)    #=> "11000000111001"
  # 12345.to_s(8)    #=> "30071"
  # 12345.to_s(10)   #=> "12345"
  # 12345.to_s(16)   #=> "3039"
  # 12345.to_s(36)   #=> "9ix"
  # 78546939656932.to_s(36)  #=> "rubyrules"
  # ```
  #
  #
  # Also aliased as:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Integer.html#method-i-inspect)
  sig {params(base: Integer).returns(String)}
  def to_s(base=10); end

  # Returns `int` truncated (toward zero) to a precision of `ndigits` decimal
  # digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns `self` when `ndigits` is zero or positive.
  #
  # ```ruby
  # 1.truncate           #=> 1
  # 1.truncate(2)        #=> 1
  # 18.truncate(-1)      #=> 10
  # (-18).truncate(-1)   #=> -10
  # ```
  sig {params(ndigits: Integer).returns(Integer)}
  def truncate(ndigits=0); end

  # Iterates the given block, passing in integer values from `int` up to and
  # including `limit`.
  #
  # If no block is given, an
  # [`Enumerator`](https://docs.ruby-lang.org/en/2.7.0/Enumerator.html) is
  # returned instead.
  #
  # ```ruby
  # 5.upto(10) {|i| print i, " " }   #=> 5 6 7 8 9 10
  # ```
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Enumerator[Integer])
  end
  def upto(arg0, &blk); end

  sig {returns(T::Boolean)}
  def zero?(); end

  # Bitwise OR.
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def |(arg0); end

  # One's complement: returns a number where each bit is flipped.
  #
  # Inverts the bits in an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html). As integers
  # are conceptually of infinite length, the result acts as if it had an
  # infinite number of one bits to the left. In hex representations, this is
  # displayed as two periods to the left of the digits.
  #
  # ```ruby
  # sprintf("%X", ~0x1122334455)    #=> "..FEEDDCCBBAA"
  # ```
  sig {returns(Integer)}
  def ~(); end

  # Returns the integer square root of the non-negative integer `n`, i.e. the
  # largest non-negative integer less than or equal to the square root of `n`.
  #
  # ```ruby
  # Integer.sqrt(0)        #=> 0
  # Integer.sqrt(1)        #=> 1
  # Integer.sqrt(24)       #=> 4
  # Integer.sqrt(25)       #=> 5
  # Integer.sqrt(10**400)  #=> 10**200
  # ```
  #
  # Equivalent to `Math.sqrt(n).floor`, except that the result of the latter
  # code may differ from the true value due to the limited precision of floating
  # point arithmetic.
  #
  # ```ruby
  # Integer.sqrt(10**46)     #=> 100000000000000000000000
  # Math.sqrt(10**46).floor  #=>  99999999999999991611392 (!)
  # ```
  #
  # If `n` is not an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html), it is
  # converted to an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) first. If `n`
  # is negative, a
  # [`Math::DomainError`](https://docs.ruby-lang.org/en/2.7.0/Math/DomainError.html)
  # is raised.
  def self.sqrt(_); end
end

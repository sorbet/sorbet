# typed: __STDLIB_INTERNAL

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
class Rational < Numeric
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
  def %(arg0); end

  # Performs multiplication.
  #
  # ```ruby
  # Rational(2, 3)  * Rational(2, 3)   #=> (4/9)
  # Rational(900)   * Rational(1)      #=> (900/1)
  # Rational(-2, 9) * Rational(-9, 2)  #=> (1/1)
  # Rational(9, 8)  * 4                #=> (9/2)
  # Rational(20, 9) * 9.8              #=> 21.77777777777778
  # ```
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
  def *(arg0); end

  # Performs exponentiation.
  #
  # ```ruby
  # Rational(2)    ** Rational(3)     #=> (8/1)
  # Rational(10)   ** -2              #=> (1/100)
  # Rational(10)   ** -2.0            #=> 0.01
  # Rational(-4)   ** Rational(1, 2)  #=> (0.0+2.0i)
  # Rational(1, 2) ** 0               #=> (1/1)
  # Rational(1, 2) ** 0.0             #=> 1.0
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
  def **(arg0); end

  # Performs addition.
  #
  # ```ruby
  # Rational(2, 3)  + Rational(2, 3)   #=> (4/3)
  # Rational(900)   + Rational(1)      #=> (901/1)
  # Rational(-2, 9) + Rational(-9, 2)  #=> (-85/18)
  # Rational(9, 8)  + 4                #=> (41/8)
  # Rational(20, 9) + 9.8              #=> 12.022222222222222
  # ```
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
  def +(arg0); end

  sig {returns(Rational)}
  def +@(); end

  # Performs subtraction.
  #
  # ```ruby
  # Rational(2, 3)  - Rational(2, 3)   #=> (0/1)
  # Rational(900)   - Rational(1)      #=> (899/1)
  # Rational(-2, 9) - Rational(-9, 2)  #=> (77/18)
  # Rational(9, 8)  - 4                #=> (-23/8)
  # Rational(20, 9) - 9.8              #=> -7.577777777777778
  # ```
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
  def -(arg0); end

  # Negates `rat`.
  sig {returns(Rational)}
  def -@(); end

  # Performs division.
  #
  # ```ruby
  # Rational(2, 3)  / Rational(2, 3)   #=> (1/1)
  # Rational(900)   / Rational(1)      #=> (900/1)
  # Rational(-2, 9) / Rational(-9, 2)  #=> (4/81)
  # Rational(9, 8)  / 4                #=> (9/32)
  # Rational(20, 9) / 9.8              #=> 0.22675736961451246
  # ```
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
  def /(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  # Returns -1, 0, or +1 depending on whether `rational` is less than, equal to,
  # or greater than `numeric`.
  #
  # `nil` is returned if the two values are incomparable.
  #
  # ```ruby
  # Rational(2, 3) <=> Rational(2, 3)  #=> 0
  # Rational(5)    <=> 5               #=> 0
  # Rational(2, 3) <=> Rational(1, 3)  #=> 1
  # Rational(1, 3) <=> 1               #=> -1
  # Rational(1, 3) <=> 0.3             #=> 1
  #
  # Rational(1, 3) <=> "0.3"           #=> nil
  # ```
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
  sig { params(arg0: T.anything).returns(NilClass) }
  def <=>(arg0); end

  # Returns `true` if `rat` equals `object` numerically.
  #
  # ```ruby
  # Rational(2, 3)  == Rational(2, 3)   #=> true
  # Rational(5)     == 5                #=> true
  # Rational(0)     == 0.0              #=> true
  # Rational('1/3') == 0.33             #=> false
  # Rational('1/2') == '1/2'            #=> false
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  # Returns the absolute value of `rat`.
  #
  # ```ruby
  # (1/2r).abs    #=> (1/2)
  # (-1/2r).abs   #=> (1/2)
  # ```
  #
  # [`Rational#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Rational.html#method-i-magnitude)
  # is an alias for
  # [`Rational#abs`](https://docs.ruby-lang.org/en/2.7.0/Rational.html#method-i-abs).
  sig {returns(Rational)}
  def abs(); end

  sig {returns(Rational)}
  def abs2(); end

  sig {returns(Numeric)}
  def angle(); end

  sig {returns(Numeric)}
  def arg(); end

  # Returns the smallest number greater than or equal to `rat` with a precision
  # of `ndigits` decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a rational when `ndigits` is positive, otherwise returns an integer.
  #
  # ```ruby
  # Rational(3).ceil      #=> 3
  # Rational(2, 3).ceil   #=> 1
  # Rational(-3, 2).ceil  #=> -1
  #
  #   #    decimal      -  1  2  3 . 4  5  6
  #   #                   ^  ^  ^  ^   ^  ^
  #   #   precision      -3 -2 -1  0  +1 +2
  #
  # Rational('-123.456').ceil(+1).to_f  #=> -123.4
  # Rational('-123.456').ceil(-1)       #=> -120
  # ```
  sig {returns(Integer)}
  sig do
    params(
        digits: Integer,
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns([Rational, Rational])
  end
  sig do
    params(
        arg0: Float,
    )
    .returns([Float, Float])
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns([Rational, Rational])
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns([Numeric, Numeric])
  end
  def coerce(arg0); end

  sig {returns(Rational)}
  def conj(); end

  sig {returns(Rational)}
  def conjugate(); end

  # Returns the denominator (always positive).
  #
  # ```ruby
  # Rational(7).denominator             #=> 1
  # Rational(7, 1).denominator          #=> 1
  # Rational(9, -4).denominator         #=> 4
  # Rational(-2, -10).denominator       #=> 5
  # ```
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

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def divmod(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def equal?(arg0); end

  # Performs division and returns the value as a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html).
  #
  # ```ruby
  # Rational(2, 3).fdiv(1)       #=> 0.6666666666666666
  # Rational(2, 3).fdiv(0.5)     #=> 1.3333333333333333
  # Rational(2).fdiv(3)          #=> 0.6666666666666666
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Float)
  end
  def fdiv(arg0); end

  # Returns the largest number less than or equal to `rat` with a precision of
  # `ndigits` decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a rational when `ndigits` is positive, otherwise returns an integer.
  #
  # ```ruby
  # Rational(3).floor      #=> 3
  # Rational(2, 3).floor   #=> 0
  # Rational(-3, 2).floor  #=> -2
  #
  #   #    decimal      -  1  2  3 . 4  5  6
  #   #                   ^  ^  ^  ^   ^  ^
  #   #   precision      -3 -2 -1  0  +1 +2
  #
  # Rational('-123.456').floor(+1).to_f  #=> -123.5
  # Rational('-123.456').floor(-1)       #=> -130
  # ```
  sig {returns(Integer)}
  sig do
    params(
        digits: Integer,
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  # Returns the value as a string for inspection.
  #
  # ```ruby
  # Rational(2).inspect      #=> "(2/1)"
  # Rational(-8, 6).inspect  #=> "(-4/3)"
  # Rational('1/2').inspect  #=> "(1/2)"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the absolute value of `rat`.
  #
  # ```ruby
  # (1/2r).abs    #=> (1/2)
  # (-1/2r).abs   #=> (1/2)
  # ```
  #
  # [`Rational#magnitude`](https://docs.ruby-lang.org/en/2.7.0/Rational.html#method-i-magnitude)
  # is an alias for
  # [`Rational#abs`](https://docs.ruby-lang.org/en/2.7.0/Rational.html#method-i-abs).
  def magnitude; end

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
  def modulo(arg0); end

  # Returns `true` if `rat` is less than 0.
  def negative?; end

  # Returns the numerator.
  #
  # ```ruby
  # Rational(7).numerator        #=> 7
  # Rational(7, 1).numerator     #=> 7
  # Rational(9, -4).numerator    #=> -9
  # Rational(-2, -10).numerator  #=> 1
  # ```
  sig {returns(Integer)}
  def numerator(); end

  sig {returns(Numeric)}
  def phase(); end

  # Returns `true` if `rat` is greater than 0.
  def positive?; end

  # Performs division.
  #
  # ```ruby
  # Rational(2, 3)  / Rational(2, 3)   #=> (1/1)
  # Rational(900)   / Rational(1)      #=> (900/1)
  # Rational(-2, 9) / Rational(-9, 2)  #=> (4/81)
  # Rational(9, 8)  / 4                #=> (9/32)
  # Rational(20, 9) / 9.8              #=> 0.22675736961451246
  # ```
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

  # Returns a simpler approximation of the value if the optional argument `eps`
  # is given (rat-|eps| <= result <= rat+|eps|), self otherwise.
  #
  # ```ruby
  # r = Rational(5033165, 16777216)
  # r.rationalize                    #=> (5033165/16777216)
  # r.rationalize(Rational('0.01'))  #=> (3/10)
  # r.rationalize(Rational('0.1'))   #=> (1/3)
  # ```
  sig {returns(Rational)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Rational)
  end
  def rationalize(arg0=T.unsafe(nil)); end

  sig {returns(Rational)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  # Returns `rat` rounded to the nearest value with a precision of `ndigits`
  # decimal digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a rational when `ndigits` is positive, otherwise returns an integer.
  #
  # ```ruby
  # Rational(3).round      #=> 3
  # Rational(2, 3).round   #=> 1
  # Rational(-3, 2).round  #=> -2
  #
  #   #    decimal      -  1  2  3 . 4  5  6
  #   #                   ^  ^  ^  ^   ^  ^
  #   #   precision      -3 -2 -1  0  +1 +2
  #
  # Rational('-123.456').round(+1).to_f  #=> -123.5
  # Rational('-123.456').round(-1)       #=> -120
  # ```
  #
  # The optional `half` keyword argument is available similar to
  # [`Float#round`](https://docs.ruby-lang.org/en/2.7.0/Float.html#method-i-round).
  #
  # ```ruby
  # Rational(25, 100).round(1, half: :up)    #=> (3/10)
  # Rational(25, 100).round(1, half: :down)  #=> (1/5)
  # Rational(25, 100).round(1, half: :even)  #=> (1/5)
  # Rational(35, 100).round(1, half: :up)    #=> (2/5)
  # Rational(35, 100).round(1, half: :down)  #=> (3/10)
  # Rational(35, 100).round(1, half: :even)  #=> (2/5)
  # Rational(-25, 100).round(1, half: :up)   #=> (-3/10)
  # Rational(-25, 100).round(1, half: :down) #=> (-1/5)
  # Rational(-25, 100).round(1, half: :even) #=> (-1/5)
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Numeric)
  end
  def round(arg0=T.unsafe(nil)); end

  sig {returns(Complex)}
  def to_c(); end

  # Returns the value as a
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html).
  #
  # ```ruby
  # Rational(2).to_f      #=> 2.0
  # Rational(9, 4).to_f   #=> 2.25
  # Rational(-3, 4).to_f  #=> -0.75
  # Rational(20, 3).to_f  #=> 6.666666666666667
  # ```
  sig {returns(Float)}
  def to_f(); end

  # Returns the truncated value as an integer.
  #
  # Equivalent to
  # [`Rational#truncate`](https://docs.ruby-lang.org/en/2.7.0/Rational.html#method-i-truncate).
  #
  # ```ruby
  # Rational(2, 3).to_i    #=> 0
  # Rational(3).to_i       #=> 3
  # Rational(300.6).to_i   #=> 300
  # Rational(98, 71).to_i  #=> 1
  # Rational(-31, 2).to_i  #=> -15
  # ```
  sig {returns(Integer)}
  def to_i(); end

  # Returns self.
  #
  # ```ruby
  # Rational(2).to_r      #=> (2/1)
  # Rational(-8, 6).to_r  #=> (-4/3)
  # ```
  sig {returns(Rational)}
  def to_r(); end

  # Returns the value as a string.
  #
  # ```ruby
  # Rational(2).to_s      #=> "2/1"
  # Rational(-8, 6).to_s  #=> "-4/3"
  # Rational('1/2').to_s  #=> "1/2"
  # ```
  sig {returns(String)}
  def to_s(); end

  # Returns `rat` truncated (toward zero) to a precision of `ndigits` decimal
  # digits (default: 0).
  #
  # When the precision is negative, the returned value is an integer with at
  # least `ndigits.abs` trailing zeros.
  #
  # Returns a rational when `ndigits` is positive, otherwise returns an integer.
  #
  # ```ruby
  # Rational(3).truncate      #=> 3
  # Rational(2, 3).truncate   #=> 0
  # Rational(-3, 2).truncate  #=> -1
  #
  #   #    decimal      -  1  2  3 . 4  5  6
  #   #                   ^  ^  ^  ^   ^  ^
  #   #   precision      -3 -2 -1  0  +1 +2
  #
  # Rational('-123.456').truncate(+1).to_f  #=> -123.4
  # Rational('-123.456').truncate(-1)       #=> -120
  # ```
  sig {returns(Integer)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
  end
  def truncate(arg0=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def zero?(); end
end

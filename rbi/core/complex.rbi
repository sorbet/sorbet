# typed: __STDLIB_INTERNAL

# A complex number can be represented as a paired real number with imaginary
# unit; a+bi. Where a is real part, b is imaginary part and i is imaginary unit.
# Real a equals complex a+0i mathematically.
#
# [`Complex`](https://docs.ruby-lang.org/en/2.6.0/Complex.html) object can be
# created as literal, and also by using Kernel#Complex,
# [`Complex::rect`](https://docs.ruby-lang.org/en/2.6.0/Complex.html#method-c-rect),
# [`Complex::polar`](https://docs.ruby-lang.org/en/2.6.0/Complex.html#method-c-polar)
# or [`to_c`](https://docs.ruby-lang.org/en/2.6.0/Complex.html#method-i-to_c)
# method.
#
# ```ruby
# 2+1i                 #=> (2+1i)
# Complex(1)           #=> (1+0i)
# Complex(2, 3)        #=> (2+3i)
# Complex.polar(2, 3)  #=> (-1.9799849932008908+0.2822400161197344i)
# 3.to_c               #=> (3+0i)
# ```
#
# You can also create complex object from floating-point numbers or strings.
#
# ```ruby
# Complex(0.3)         #=> (0.3+0i)
# Complex('0.3-0.5i')  #=> (0.3-0.5i)
# Complex('2/3+3/4i')  #=> ((2/3)+(3/4)*i)
# Complex('1@2')       #=> (-0.4161468365471424+0.9092974268256817i)
#
# 0.3.to_c             #=> (0.3+0i)
# '0.3-0.5i'.to_c      #=> (0.3-0.5i)
# '2/3+3/4i'.to_c      #=> ((2/3)+(3/4)*i)
# '1@2'.to_c           #=> (-0.4161468365471424+0.9092974268256817i)
# ```
#
# A complex object is either an exact or an inexact number.
#
# ```ruby
# Complex(1, 1) / 2    #=> ((1/2)+(1/2)*i)
# Complex(1, 1) / 2.0  #=> (0.5+0.5i)
# ```
class Complex < Numeric
  # The imaginary unit.
  I = T.let(T.unsafe(nil), Complex)

  # Performs multiplication.
  #
  # ```ruby
  # Complex(2, 3)  * Complex(2, 3)   #=> (-5+12i)
  # Complex(900)   * Complex(1)      #=> (900+0i)
  # Complex(-2, 9) * Complex(-9, 2)  #=> (0-85i)
  # Complex(9, 8)  * 4               #=> (36+32i)
  # Complex(20, 9) * 9.8             #=> (196.0+88.2i)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
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
  # Complex('i') ** 2              #=> (-1+0i)
  # Complex(-8) ** Rational(1, 3)  #=> (1.0000000000000002+1.7320508075688772i)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
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
  # Complex(2, 3)  + Complex(2, 3)   #=> (4+6i)
  # Complex(900)   + Complex(1)      #=> (901+0i)
  # Complex(-2, 9) + Complex(-9, 2)  #=> (-11+11i)
  # Complex(9, 8)  + 4               #=> (13+8i)
  # Complex(20, 9) + 9.8             #=> (29.8+9i)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end

  sig {returns(Complex)}
  def +@(); end

  # Performs subtraction.
  #
  # ```ruby
  # Complex(2, 3)  - Complex(2, 3)   #=> (0+0i)
  # Complex(900)   - Complex(1)      #=> (899+0i)
  # Complex(-2, 9) - Complex(-9, 2)  #=> (7+7i)
  # Complex(9, 8)  - 4               #=> (5+8i)
  # Complex(20, 9) - 9.8             #=> (10.2+9i)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end

  # Returns negation of the value.
  #
  # ```ruby
  # -Complex(1, 2)  #=> (-1-2i)
  # ```
  sig {returns(Complex)}
  def -@(); end

  # Performs division.
  #
  # ```ruby
  # Complex(2, 3)  / Complex(2, 3)   #=> ((1/1)+(0/1)*i)
  # Complex(900)   / Complex(1)      #=> ((900/1)+(0/1)*i)
  # Complex(-2, 9) / Complex(-9, 2)  #=> ((36/85)-(77/85)*i)
  # Complex(9, 8)  / 4               #=> ((9/4)+(2/1)*i)
  # Complex(20, 9) / 9.8             #=> (2.0408163265306123+0.9183673469387754i)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def /(arg0); end

  # Returns true if cmp equals object numerically.
  #
  # ```ruby
  # Complex(2, 3)  == Complex(2, 3)   #=> true
  # Complex(5)     == 5               #=> true
  # Complex(0)     == 0.0             #=> true
  # Complex('1/3') == 0.33            #=> false
  # Complex('1/2') == '1/2'           #=> false
  # ```
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  # Returns the absolute part of its polar form.
  #
  # ```ruby
  # Complex(-1).abs         #=> 1
  # Complex(3.0, -4.0).abs  #=> 5.0
  # ```
  sig {returns(Numeric)}
  def abs(); end

  # Returns square of the absolute value.
  #
  # ```ruby
  # Complex(-1).abs2         #=> 1
  # Complex(3.0, -4.0).abs2  #=> 25.0
  # ```
  sig {returns(Numeric)}
  def abs2(); end

  # Returns the angle part of its polar form.
  #
  # ```ruby
  # Complex.polar(3, Math::PI/2).arg  #=> 1.5707963267948966
  # ```
  sig {returns(Float)}
  def angle(); end

  # Returns the angle part of its polar form.
  #
  # ```ruby
  # Complex.polar(3, Math::PI/2).arg  #=> 1.5707963267948966
  # ```
  sig {returns(Float)}
  def arg(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns([Complex, Complex])
  end
  def coerce(arg0); end

  # Returns the complex conjugate.
  #
  # ```ruby
  # Complex(1, 2).conjugate  #=> (1-2i)
  # ```
  sig {returns(Complex)}
  def conj(); end

  # Returns the complex conjugate.
  #
  # ```ruby
  # Complex(1, 2).conjugate  #=> (1-2i)
  # ```
  sig {returns(Complex)}
  def conjugate(); end

  # Returns the denominator (lcm of both denominator - real and imag).
  #
  # See numerator.
  sig {returns(Integer)}
  def denominator(); end

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

  # Returns `true` if `cmp`'s real and imaginary parts are both finite numbers,
  # otherwise returns `false`.
  def finite?; end

  # Performs division as each part is a float, never returns a float.
  #
  # ```ruby
  # Complex(11, 22).fdiv(3)  #=> (3.6666666666666665+7.333333333333333i)
  # ```
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Complex)
  end
  def fdiv(arg0); end

  sig {returns(Integer)}
  def hash(); end

  # Returns the imaginary part.
  #
  # ```ruby
  # Complex(7).imaginary      #=> 0
  # Complex(9, -4).imaginary  #=> -4
  # ```
  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def imag(); end

  # Returns the imaginary part.
  #
  # ```ruby
  # Complex(7).imaginary      #=> 0
  # Complex(9, -4).imaginary  #=> -4
  # ```
  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def imaginary(); end

  # Returns `1` if `cmp`'s real or imaginary part is an infinite number,
  # otherwise returns `nil`.
  #
  # ```ruby
  # For example:
  #
  #    (1+1i).infinite?                   #=> nil
  #    (Float::INFINITY + 1i).infinite?   #=> 1
  # ```
  def infinite?; end

  # Returns the value as a string for inspection.
  #
  # ```ruby
  # Complex(2).inspect                       #=> "(2+0i)"
  # Complex('-8/6').inspect                  #=> "((-4/3)+0i)"
  # Complex('1/2i').inspect                  #=> "(0+(1/2)*i)"
  # Complex(0, Float::INFINITY).inspect      #=> "(0+Infinity*i)"
  # Complex(Float::NAN, Float::NAN).inspect  #=> "(NaN+NaN*i)"
  # ```
  sig {returns(String)}
  def inspect(); end

  # Returns the absolute part of its polar form.
  #
  # ```ruby
  # Complex(-1).abs         #=> 1
  # Complex(3.0, -4.0).abs  #=> 5.0
  # ```
  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def magnitude(); end

  # Returns the numerator.
  #
  # ```
  #     1   2       3+4i  <-  numerator
  #     - + -i  ->  ----
  #     2   3        6    <-  denominator
  #
  # c = Complex('1/2+2/3i')  #=> ((1/2)+(2/3)*i)
  # n = c.numerator          #=> (3+4i)
  # d = c.denominator        #=> 6
  # n / d                    #=> ((1/2)+(2/3)*i)
  # Complex(Rational(n.real, d), Rational(n.imag, d))
  #                          #=> ((1/2)+(2/3)*i)
  # ```
  #
  # See denominator.
  sig {returns(Complex)}
  def numerator(); end

  # Returns the angle part of its polar form.
  #
  # ```ruby
  # Complex.polar(3, Math::PI/2).arg  #=> 1.5707963267948966
  # ```
  sig {returns(Float)}
  def phase(); end

  # Returns an array; [cmp.abs, cmp.arg].
  #
  # ```ruby
  # Complex(1, 2).polar  #=> [2.23606797749979, 1.1071487177940904]
  # ```
  sig {returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])}
  def polar(); end

  # Performs division.
  #
  # ```ruby
  # Complex(2, 3)  / Complex(2, 3)   #=> ((1/1)+(0/1)*i)
  # Complex(900)   / Complex(1)      #=> ((900/1)+(0/1)*i)
  # Complex(-2, 9) / Complex(-9, 2)  #=> ((36/85)-(77/85)*i)
  # Complex(9, 8)  / 4               #=> ((9/4)+(2/1)*i)
  # Complex(20, 9) / 9.8             #=> (2.0408163265306123+0.9183673469387754i)
  # ```
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
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

  # Returns the value as a rational if possible (the imaginary part should be
  # exactly zero).
  #
  # ```ruby
  # Complex(1.0/3, 0).rationalize  #=> (1/3)
  # Complex(1, 0.0).rationalize    # RangeError
  # Complex(1, 2).rationalize      # RangeError
  # ```
  #
  # See to\_r.
  sig {returns(Rational)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Rational)
  end
  def rationalize(arg0=T.unsafe(nil)); end

  # Returns the real part.
  #
  # ```ruby
  # Complex(7).real      #=> 7
  # Complex(9, -4).real  #=> 9
  # ```
  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def real(); end

  # Returns false.
  sig {returns(FalseClass)}
  def real?(); end

  # Returns an array; [cmp.real, cmp.imag].
  #
  # ```ruby
  # Complex(1, 2).rectangular  #=> [1, 2]
  # ```
  sig {returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])}
  def rect(); end

  # Returns an array; [cmp.real, cmp.imag].
  #
  # ```ruby
  # Complex(1, 2).rectangular  #=> [1, 2]
  # ```
  sig {returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])}
  def rectangular(); end

  # Returns self.
  #
  # ```ruby
  # Complex(2).to_c      #=> (2+0i)
  # Complex(-8, 6).to_c  #=> (-8+6i)
  # ```
  sig {returns(Complex)}
  def to_c(); end

  # Returns the value as a float if possible (the imaginary part should be
  # exactly zero).
  #
  # ```ruby
  # Complex(1, 0).to_f    #=> 1.0
  # Complex(1, 0.0).to_f  # RangeError
  # Complex(1, 2).to_f    # RangeError
  # ```
  sig {returns(Float)}
  def to_f(); end

  # Returns the value as an integer if possible (the imaginary part should be
  # exactly zero).
  #
  # ```ruby
  # Complex(1, 0).to_i    #=> 1
  # Complex(1, 0.0).to_i  # RangeError
  # Complex(1, 2).to_i    # RangeError
  # ```
  sig {returns(Integer)}
  def to_i(); end

  # Returns the value as a rational if possible (the imaginary part should be
  # exactly zero).
  #
  # ```ruby
  # Complex(1, 0).to_r    #=> (1/1)
  # Complex(1, 0.0).to_r  # RangeError
  # Complex(1, 2).to_r    # RangeError
  # ```
  #
  # See rationalize.
  sig {returns(Rational)}
  def to_r(); end

  # Returns the value as a string.
  #
  # ```ruby
  # Complex(2).to_s                       #=> "2+0i"
  # Complex('-8/6').to_s                  #=> "-4/3+0i"
  # Complex('1/2i').to_s                  #=> "0+1/2i"
  # Complex(0, Float::INFINITY).to_s      #=> "0+Infinity*i"
  # Complex(Float::NAN, Float::NAN).to_s  #=> "NaN+NaN*i"
  # ```
  sig {returns(String)}
  def to_s(); end

  sig {returns(T::Boolean)}
  def zero?(); end

  # Returns a complex object which denotes the given polar form.
  #
  # ```ruby
  # Complex.polar(3, 0)            #=> (3.0+0.0i)
  # Complex.polar(3, Math::PI/2)   #=> (1.836909530733566e-16+3.0i)
  # Complex.polar(3, Math::PI)     #=> (-3.0+3.673819061467132e-16i)
  # Complex.polar(3, -Math::PI/2)  #=> (1.836909530733566e-16-3.0i)
  # ```
  def self.polar(*_); end

  # Returns a complex object which denotes the given rectangular form.
  #
  # ```ruby
  # Complex.rectangular(1, 2)  #=> (1+2i)
  # ```
  def self.rect(*_); end

  # Returns a complex object which denotes the given rectangular form.
  #
  # ```ruby
  # Complex.rectangular(1, 2)  #=> (1+2i)
  # ```
  def self.rectangular(*_); end
end

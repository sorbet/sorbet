# typed: __STDLIB_INTERNAL

# The [`Math`](https://docs.ruby-lang.org/en/2.6.0/Math.html) module contains
# module functions for basic trigonometric and transcendental functions. See
# class [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html) for a list of
# constants that define Ruby's floating point accuracy.
#
# Domains and codomains are given only for real (not complex) numbers.
module Math
  # Definition of the mathematical constant
  # [`E`](https://docs.ruby-lang.org/en/2.6.0/Math.html#E) (e) as a
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html) number.
  E = T.let(T.unsafe(nil), Float)
  # Definition of the mathematical constant
  # [`PI`](https://docs.ruby-lang.org/en/2.6.0/Math.html#PI) as a
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html) number.
  PI = T.let(T.unsafe(nil), Float)

  # Computes the arc cosine of `x`. Returns 0..PI.
  #
  # Domain: [-1, 1]
  #
  # Codomain: [0, PI]
  #
  # ```ruby
  # Math.acos(0) == Math::PI/2  #=> true
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.acos(x); end

  # Computes the inverse hyperbolic cosine of `x`.
  #
  # Domain: [1, INFINITY)
  #
  # Codomain: [0, INFINITY)
  #
  # ```ruby
  # Math.acosh(1) #=> 0.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.acosh(x); end

  # Computes the arc sine of `x`. Returns -PI/2..PI/2.
  #
  # Domain: [-1, -1]
  #
  # Codomain: [-PI/2, PI/2]
  #
  # ```ruby
  # Math.asin(1) == Math::PI/2  #=> true
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.asin(x); end

  # Computes the inverse hyperbolic sine of `x`.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # Math.asinh(1) #=> 0.881373587019543
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.asinh(x); end

  # Computes the arc tangent of `x`. Returns -PI/2..PI/2.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-PI/2, PI/2)
  #
  # ```ruby
  # Math.atan(0) #=> 0.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.atan(x); end

  # Computes the arc tangent given `y` and `x`. Returns a
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html) in the range
  # -PI..PI. Return value is a angle in radians between the positive x-axis of
  # cartesian plane and the point given by the coordinates (`x`, `y`) on it.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: [-PI, PI]
  #
  # ```ruby
  # Math.atan2(-0.0, -1.0) #=> -3.141592653589793
  # Math.atan2(-1.0, -1.0) #=> -2.356194490192345
  # Math.atan2(-1.0, 0.0)  #=> -1.5707963267948966
  # Math.atan2(-1.0, 1.0)  #=> -0.7853981633974483
  # Math.atan2(-0.0, 1.0)  #=> -0.0
  # Math.atan2(0.0, 1.0)   #=> 0.0
  # Math.atan2(1.0, 1.0)   #=> 0.7853981633974483
  # Math.atan2(1.0, 0.0)   #=> 1.5707963267948966
  # Math.atan2(1.0, -1.0)  #=> 2.356194490192345
  # Math.atan2(0.0, -1.0)  #=> 3.141592653589793
  # Math.atan2(INFINITY, INFINITY)   #=> 0.7853981633974483
  # Math.atan2(INFINITY, -INFINITY)  #=> 2.356194490192345
  # Math.atan2(-INFINITY, INFINITY)  #=> -0.7853981633974483
  # Math.atan2(-INFINITY, -INFINITY) #=> -2.356194490192345
  # ```
  sig do
    params(
        y: T.any(Integer, Float, Rational, BigDecimal),
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.atan2(y, x); end

  # Computes the inverse hyperbolic tangent of `x`.
  #
  # Domain: (-1, 1)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # Math.atanh(1) #=> Infinity
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.atanh(x); end

  # Returns the cube root of `x`.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # -9.upto(9) {|x|
  #   p [x, Math.cbrt(x), Math.cbrt(x)**3]
  # }
  # #=> [-9, -2.0800838230519, -9.0]
  # #   [-8, -2.0, -8.0]
  # #   [-7, -1.91293118277239, -7.0]
  # #   [-6, -1.81712059283214, -6.0]
  # #   [-5, -1.7099759466767, -5.0]
  # #   [-4, -1.5874010519682, -4.0]
  # #   [-3, -1.44224957030741, -3.0]
  # #   [-2, -1.25992104989487, -2.0]
  # #   [-1, -1.0, -1.0]
  # #   [0, 0.0, 0.0]
  # #   [1, 1.0, 1.0]
  # #   [2, 1.25992104989487, 2.0]
  # #   [3, 1.44224957030741, 3.0]
  # #   [4, 1.5874010519682, 4.0]
  # #   [5, 1.7099759466767, 5.0]
  # #   [6, 1.81712059283214, 6.0]
  # #   [7, 1.91293118277239, 7.0]
  # #   [8, 2.0, 8.0]
  # #   [9, 2.0800838230519, 9.0]
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.cbrt(x); end

  # Computes the cosine of `x` (expressed in radians). Returns a
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html) in the range
  # -1.0..1.0.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: [-1, 1]
  #
  # ```ruby
  # Math.cos(Math::PI) #=> -1.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.cos(x); end

  # Computes the hyperbolic cosine of `x` (expressed in radians).
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: [1, INFINITY)
  #
  # ```ruby
  # Math.cosh(0) #=> 1.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.cosh(x); end

  # Calculates the error function of `x`.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-1, 1)
  #
  # ```ruby
  # Math.erf(0) #=> 0.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.erf(x); end

  # Calculates the complementary error function of x.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (0, 2)
  #
  # ```ruby
  # Math.erfc(0) #=> 1.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.erfc(x); end

  # Returns e\*\*x.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (0, INFINITY)
  #
  # ```ruby
  # Math.exp(0)       #=> 1.0
  # Math.exp(1)       #=> 2.718281828459045
  # Math.exp(1.5)     #=> 4.4816890703380645
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.exp(x); end

  # Returns a two-element array containing the normalized fraction (a
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html)) and exponent (an
  # [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html)) of `x`.
  #
  # ```ruby
  # fraction, exponent = Math.frexp(1234)   #=> [0.6025390625, 11]
  # fraction * 2**exponent                  #=> 1234.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def self.frexp(x); end

  # Calculates the gamma function of x.
  #
  # Note that gamma(n) is same as fact(n-1) for integer n > 0. However gamma(n)
  # returns float and can be an approximation.
  #
  # ```ruby
  # def fact(n) (1..n).inject(1) {|r,i| r*i } end
  # 1.upto(26) {|i| p [i, Math.gamma(i), fact(i-1)] }
  # #=> [1, 1.0, 1]
  # #   [2, 1.0, 1]
  # #   [3, 2.0, 2]
  # #   [4, 6.0, 6]
  # #   [5, 24.0, 24]
  # #   [6, 120.0, 120]
  # #   [7, 720.0, 720]
  # #   [8, 5040.0, 5040]
  # #   [9, 40320.0, 40320]
  # #   [10, 362880.0, 362880]
  # #   [11, 3628800.0, 3628800]
  # #   [12, 39916800.0, 39916800]
  # #   [13, 479001600.0, 479001600]
  # #   [14, 6227020800.0, 6227020800]
  # #   [15, 87178291200.0, 87178291200]
  # #   [16, 1307674368000.0, 1307674368000]
  # #   [17, 20922789888000.0, 20922789888000]
  # #   [18, 355687428096000.0, 355687428096000]
  # #   [19, 6.402373705728e+15, 6402373705728000]
  # #   [20, 1.21645100408832e+17, 121645100408832000]
  # #   [21, 2.43290200817664e+18, 2432902008176640000]
  # #   [22, 5.109094217170944e+19, 51090942171709440000]
  # #   [23, 1.1240007277776077e+21, 1124000727777607680000]
  # #   [24, 2.5852016738885062e+22, 25852016738884976640000]
  # #   [25, 6.204484017332391e+23, 620448401733239439360000]
  # #   [26, 1.5511210043330954e+25, 15511210043330985984000000]
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.gamma(x); end

  # Returns sqrt(x\*\*2 + y\*\*2), the hypotenuse of a right-angled triangle
  # with sides `x` and `y`.
  #
  # ```ruby
  # Math.hypot(3, 4)   #=> 5.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
        y: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.hypot(x, y); end

  # Returns the value of `fraction`\*(2\*\*`exponent`).
  #
  # ```ruby
  # fraction, exponent = Math.frexp(1234)
  # Math.ldexp(fraction, exponent)   #=> 1234.0
  # ```
  sig do
    params(
        fraction: T.any(Integer, Float, Rational, BigDecimal),
        exponent: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.ldexp(fraction, exponent); end

  # Calculates the logarithmic gamma of `x` and the sign of gamma of `x`.
  #
  # [`Math.lgamma(x)`](https://docs.ruby-lang.org/en/2.6.0/Math.html#method-c-lgamma)
  # is same as
  #
  # ```ruby
  # [Math.log(Math.gamma(x).abs), Math.gamma(x) < 0 ? -1 : 1]
  # ```
  #
  # but avoid overflow by
  # [`Math.gamma(x)`](https://docs.ruby-lang.org/en/2.6.0/Math.html#method-c-gamma)
  # for large x.
  #
  # ```ruby
  # Math.lgamma(0) #=> [Infinity, 1]
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T.any(Integer, Float))
  end
  def self.lgamma(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
        base: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.log(x, base=T.unsafe(nil)); end

  # Returns the base 10 logarithm of `x`.
  #
  # Domain: (0, INFINITY)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # Math.log10(1)       #=> 0.0
  # Math.log10(10)      #=> 1.0
  # Math.log10(10**100) #=> 100.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.log10(x); end

  # Returns the base 2 logarithm of `x`.
  #
  # Domain: (0, INFINITY)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # Math.log2(1)      #=> 0.0
  # Math.log2(2)      #=> 1.0
  # Math.log2(32768)  #=> 15.0
  # Math.log2(65536)  #=> 16.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.log2(x); end

  # Computes the sine of `x` (expressed in radians). Returns a
  # [`Float`](https://docs.ruby-lang.org/en/2.6.0/Float.html) in the range
  # -1.0..1.0.
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: [-1, 1]
  #
  # ```ruby
  # Math.sin(Math::PI/2) #=> 1.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.sin(x); end

  # Computes the hyperbolic sine of `x` (expressed in radians).
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # Math.sinh(0) #=> 0.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.sinh(x); end

  # Returns the non-negative square root of `x`.
  #
  # Domain: [0, INFINITY)
  #
  # Codomain:[0, INFINITY)
  #
  # ```ruby
  # 0.upto(10) {|x|
  #   p [x, Math.sqrt(x), Math.sqrt(x)**2]
  # }
  # #=> [0, 0.0, 0.0]
  # #   [1, 1.0, 1.0]
  # #   [2, 1.4142135623731, 2.0]
  # #   [3, 1.73205080756888, 3.0]
  # #   [4, 2.0, 4.0]
  # #   [5, 2.23606797749979, 5.0]
  # #   [6, 2.44948974278318, 6.0]
  # #   [7, 2.64575131106459, 7.0]
  # #   [8, 2.82842712474619, 8.0]
  # #   [9, 3.0, 9.0]
  # #   [10, 3.16227766016838, 10.0]
  # ```
  #
  # Note that the limited precision of floating point arithmetic might lead to
  # surprising results:
  #
  # ```ruby
  # Math.sqrt(10**46).to_i  #=> 99999999999999991611392 (!)
  # ```
  #
  # See also
  # [`BigDecimal#sqrt`](https://docs.ruby-lang.org/en/2.6.0/BigDecimal.html#method-i-sqrt)
  # and
  # [`Integer.sqrt`](https://docs.ruby-lang.org/en/2.6.0/Integer.html#method-c-sqrt).
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.sqrt(x); end

  # Computes the tangent of `x` (expressed in radians).
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-INFINITY, INFINITY)
  #
  # ```ruby
  # Math.tan(0) #=> 0.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.tan(x); end

  # Computes the hyperbolic tangent of `x` (expressed in radians).
  #
  # Domain: (-INFINITY, INFINITY)
  #
  # Codomain: (-1, 1)
  #
  # ```ruby
  # Math.tanh(0) #=> 0.0
  # ```
  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.tanh(x); end
end

# Raised when a mathematical function is evaluated outside of its domain of
# definition.
#
# For example, since `cos` returns values in the range -1..1, its inverse
# function `acos` is only defined on that interval:
#
# ```ruby
# Math.acos(42)
# ```
#
# *produces:*
#
# ```
# Math::DomainError: Numerical argument is out of domain - "acos"
# ```
class Math::DomainError < StandardError
end

# typed: __STDLIB_INTERNAL

# Provides mathematical functions.
#
# Example:
#
# ```ruby
# require "bigdecimal/math"
#
# include BigMath
#
# a = BigDecimal((PI(100)/2).to_s)
# puts sin(a,100) # => 0.99999999999999999999......e0
# ```
module BigMath
  # Computes the value of e (the base of natural logarithms) raised to the power
  # of `decimal`, to the specified number of digits of precision.
  #
  # If `decimal` is infinity, returns Infinity.
  #
  # If `decimal` is NaN, returns NaN.
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def self.exp(arg0, arg1); end

  # Computes the natural logarithm of `decimal` to the specified number of
  # digits of precision, `numeric`.
  #
  # If `decimal` is zero or negative, raises
  # [`Math::DomainError`](https://docs.ruby-lang.org/en/2.6.0/Math/DomainError.html).
  #
  # If `decimal` is positive infinity, returns Infinity.
  #
  # If `decimal` is NaN, returns NaN.
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def self.log(arg0, arg1); end

  # Computes e (the base of natural logarithms) to the specified number of
  # digits of precision, `numeric`.
  #
  # ```ruby
  # BigMath.E(10).to_s
  # #=> "0.271828182845904523536028752390026306410273e1"
  # ```
  sig do
    params(
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def E(prec); end

  # Computes the value of pi to the specified number of digits of precision,
  # `numeric`.
  #
  # ```ruby
  # BigMath.PI(10).to_s
  # #=> "0.3141592653589793238462643388813853786957412e1"
  # ```
  sig do
    params(
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def PI(prec); end

  # Computes the arctangent of `decimal` to the specified number of digits of
  # precision, `numeric`.
  #
  # If `decimal` is NaN, returns NaN.
  #
  # ```ruby
  # BigMath.atan(BigDecimal('-1'), 16).to_s
  # #=> "-0.785398163397448309615660845819878471907514682065e0"
  # ```
  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def atan(x, prec); end

  # Computes the cosine of `decimal` to the specified number of digits of
  # precision, `numeric`.
  #
  # If `decimal` is Infinity or NaN, returns NaN.
  #
  # ```ruby
  # BigMath.cos(BigMath.PI(4), 16).to_s
  # #=> "-0.999999999999999999999999999999856613163740061349e0"
  # ```
  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def cos(x, prec); end

  # Computes the sine of `decimal` to the specified number of digits of
  # precision, `numeric`.
  #
  # If `decimal` is Infinity or NaN, returns NaN.
  #
  # ```ruby
  # BigMath.sin(BigMath.PI(5)/4, 5).to_s
  # #=> "0.70710678118654752440082036563292800375e0"
  # ```
  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def sin(x, prec); end

  # Computes the square root of `decimal` to the specified number of digits of
  # precision, `numeric`.
  #
  # ```ruby
  # BigMath.sqrt(BigDecimal('2'), 16).to_s
  # #=> "0.1414213562373095048801688724e1"
  # ```
  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def sqrt(x, prec); end
end

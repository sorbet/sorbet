# typed: true

require "bigdecimal/math"

class TestWrapper
  include BigMath

  def my_e(prec)
    E(prec)
  end

  def my_pi(prec)
    PI(prec)
  end

  def my_atan(x, prec)
    atan(x, prec)
  end

  def my_cos(x, prec)
    cos(x, prec)
  end

  def my_sin(x, prec)
    sin(x, prec)
  end

  def my_sqrt(x, prec)
    sqrt(x, prec)
  end
end

BigMath.log(BigDecimal("4"), 2)
BigMath.log(4, 2)
BigMath.log(4.0, 2)

prec_cases = [BigDecimal("2"), 2, 2.0]

prec_cases.each do |prec|
  BigMath.exp(BigDecimal("4"), prec)
  BigMath.exp(4, prec)
  BigMath.exp(4.0, prec)

  BigMath.E(prec)
  TestWrapper.new.my_e(prec)

  BigMath.PI(prec)
  TestWrapper.new.my_pi(prec)

  BigMath.atan(BigDecimal("4"), prec)
  TestWrapper.new.my_atan(BigDecimal("4"), prec)

  BigMath.cos(BigDecimal("4"), prec)
  TestWrapper.new.my_cos(BigDecimal("4"), prec)

  BigMath.sin(BigDecimal("4"), prec)
  TestWrapper.new.my_sin(BigDecimal("4"), prec)

  BigMath.sqrt(BigDecimal("4"), prec)
  TestWrapper.new.my_sqrt(BigDecimal("4"), prec)
end
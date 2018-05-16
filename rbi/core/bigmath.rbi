# typed: true
module BigMath
  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(BigDecimal)
  def self.exp(arg0, arg1); end

  sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(BigDecimal)
  def self.log(arg0, arg1); end

  sig(
      prec: Integer,
  )
  .returns(BigDecimal)
  def E(prec); end

  sig(
      prec: Integer,
  )
  .returns(BigDecimal)
  def PI(prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def atan(x, prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def cos(x, prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def sin(x, prec); end

  sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def sqrt(x, prec); end
end

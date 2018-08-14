# typed: true
module BigMath
  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(BigDecimal)
  def self.exp(arg0, arg1); end

  Sorbet.sig(
      arg0: Integer,
      arg1: Integer,
  )
  .returns(BigDecimal)
  def self.log(arg0, arg1); end

  Sorbet.sig(
      prec: Integer,
  )
  .returns(BigDecimal)
  def E(prec); end

  Sorbet.sig(
      prec: Integer,
  )
  .returns(BigDecimal)
  def PI(prec); end

  Sorbet.sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def atan(x, prec); end

  Sorbet.sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def cos(x, prec); end

  Sorbet.sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def sin(x, prec); end

  Sorbet.sig(
      x: Integer,
      prec: Integer,
  )
  .returns(BigDecimal)
  def sqrt(x, prec); end
end

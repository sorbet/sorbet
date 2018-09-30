# typed: true
module BigMath
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def self.exp(arg0, arg1); end

  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def self.log(arg0, arg1); end

  sig do
    params(
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def E(prec); end

  sig do
    params(
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def PI(prec); end

  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def atan(x, prec); end

  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def cos(x, prec); end

  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def sin(x, prec); end

  sig do
    params(
        x: Integer,
        prec: Integer,
    )
    .returns(BigDecimal)
  end
  def sqrt(x, prec); end
end

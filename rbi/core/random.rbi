# typed: true
class Random < Object
  include Random::Formatter

  DEFAULT = T.let(T.unsafe(nil), Random)

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(arg0); end

  sig do
    params(
        size: Integer,
    )
    .returns(String)
  end
  def bytes(size); end

  sig do
    params(
        seed: Integer,
    )
    .void
  end
  def initialize(seed=T.unsafe(nil)); end

  sig do
    params(
        max: T.any(Integer, T::Range[Integer]),
    )
    .returns(Integer)
  end
  sig do
    params(
        max: T.any(Float, T::Range[Float]),
    )
    .returns(Float)
  end
  def rand(max=T.unsafe(nil)); end

  sig {returns(Integer)}
  def seed(); end

  sig {returns(Integer)}
  def self.new_seed(); end

  sig do
    params(
        max: Integer,
    )
    .returns(Numeric)
  end
  def self.rand(max=T.unsafe(nil)); end

  sig do
    params(
        number: Integer,
    )
    .returns(Numeric)
  end
  def self.srand(number=T.unsafe(nil)); end
end

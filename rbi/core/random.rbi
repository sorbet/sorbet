# typed: true
class Random < Object
  include Random::Formatter

  DEFAULT = T.let(T.unsafe(nil), Random)

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      size: Integer,
  )
  .returns(String)
  def bytes(size); end

  sig(
      seed: Integer,
  )
  .void
  def initialize(seed=_); end

  sig(
      max: T.any(Integer, T::Range[Integer]),
  )
  .returns(Integer)
  sig(
      max: T.any(Float, T::Range[Float]),
  )
  .returns(Float)
  def rand(max=_); end

  sig.returns(Integer)
  def seed(); end

  sig.returns(Integer)
  def self.new_seed(); end

  sig(
      max: Integer,
  )
  .returns(Numeric)
  def self.rand(max=_); end

  sig(
      number: Integer,
  )
  .returns(Numeric)
  def self.srand(number=_); end
end

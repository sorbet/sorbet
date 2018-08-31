# typed: true
class Random < Object
  include Random::Formatter

  DEFAULT = T.let(T.unsafe(nil), Random)

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  Sorbet.sig(
      size: Integer,
  )
  .returns(String)
  def bytes(size); end

  Sorbet.sig(
      seed: Integer,
  )
  .void
  def initialize(seed=T.unsafe(nil)); end

  Sorbet.sig(
      max: T.any(Integer, T::Range[Integer]),
  )
  .returns(Integer)
  Sorbet.sig(
      max: T.any(Float, T::Range[Float]),
  )
  .returns(Float)
  def rand(max=T.unsafe(nil)); end

  Sorbet.sig.returns(Integer)
  def seed(); end

  Sorbet.sig.returns(Integer)
  def self.new_seed(); end

  Sorbet.sig(
      max: Integer,
  )
  .returns(Numeric)
  def self.rand(max=T.unsafe(nil)); end

  Sorbet.sig(
      number: Integer,
  )
  .returns(Numeric)
  def self.srand(number=T.unsafe(nil)); end
end

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

module Random::Formatter
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def base64(n=nil); end

  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def hex(n=nil); end

  sig {returns(Float)}
  sig do
    params(
      n: T.nilable(Float)
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(Numeric)
    )
    .returns(Numeric)
  end
  sig do
    params(
      n: T.nilable(T::Range[Float])
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(T::Range[Integer])
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(T::Range[Numeric])
    )
    .returns(Numeric)
  end
  def rand(n=nil); end

  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(String)
  end
  def random_bytes(n=nil); end

  sig {returns(Float)}
  sig do
    params(
      n: T.nilable(Float)
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(Integer)
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(Numeric)
    )
    .returns(Numeric)
  end
  sig do
    params(
      n: T.nilable(T::Range[Float])
    )
    .returns(Float)
  end
  sig do
    params(
      n: T.nilable(T::Range[Integer])
    )
    .returns(Integer)
  end
  sig do
    params(
      n: T.nilable(T::Range[Numeric])
    )
    .returns(Numeric)
  end
  def random_number(n=nil); end

  sig do
    params(
      n: T.nilable(Integer),
      padding: T.any(FalseClass, TrueClass)
    )
    .returns(String)
  end
  def urlsafe_base64(n=nil, padding=false); end

  sig {returns(String)}
  def uuid(); end
end

# typed: core
class Regexp < Object
  EXTENDED = T.let(T.unsafe(nil), Integer)
  FIXEDENCODING = T.let(T.unsafe(nil), Integer)
  IGNORECASE = T.let(T.unsafe(nil), Integer)
  MULTILINE = T.let(T.unsafe(nil), Integer)
  NOENCODING = T.let(T.unsafe(nil), Integer)

  sig do
    params(
        arg0: T.any(String, Symbol),
    )
    .returns(String)
  end
  def self.escape(arg0); end

  sig {returns(MatchData)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(String)
  end
  def self.last_match(arg0=T.unsafe(nil)); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.nilable(Regexp))
  end
  def self.try_convert(obj); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end

  sig do
    params(
        str: T.nilable(String),
    )
    .returns(T.nilable(Integer))
  end
  def =~(str); end

  sig {returns(T::Boolean)}
  def casefold?(); end

  sig {returns(Encoding)}
  def encoding(); end

  sig {returns(T::Boolean)}
  def fixed_encoding?(); end

  sig {returns(Integer)}
  def hash(); end

  sig do
    params(
        arg0: String,
        options: BasicObject,
        kcode: String,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Regexp,
    )
    .void
  end
  def initialize(arg0, options=T.unsafe(nil), kcode=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig do
    params(
        arg0: T.nilable(String),
        arg1: Integer,
    )
    .returns(T.nilable(MatchData))
  end
  def match(arg0, arg1=T.unsafe(nil)); end

  sig {returns(T::Hash[String, T::Array[Integer]])}
  def named_captures(); end

  sig {returns(T::Array[String])}
  def names(); end

  sig {returns(Integer)}
  def options(); end

  sig {returns(String)}
  def source(); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(T.nilable(Integer))}
  def ~(); end

  sig do
    params(
        arg0: String,
        options: BasicObject,
        kcode: String,
    )
    .returns(T.self_type)
  end
  sig do
    params(
        arg0: Regexp,
    )
    .returns(T.self_type)
  end
  def self.compile(arg0, options=T.unsafe(nil), kcode=T.unsafe(nil)); end

  sig do
    params(
        arg0: T.any(String, Symbol),
    )
    .returns(String)
  end
  def self.quote(arg0); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(other); end
end

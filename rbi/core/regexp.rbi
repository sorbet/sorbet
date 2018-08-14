# typed: true
class Regexp < Object
  EXTENDED = T.let(T.unsafe(nil), Integer)
  FIXEDENCODING = T.let(T.unsafe(nil), Integer)
  IGNORECASE = T.let(T.unsafe(nil), Integer)
  MULTILINE = T.let(T.unsafe(nil), Integer)
  NOENCODING = T.let(T.unsafe(nil), Integer)

  Sorbet.sig(
      arg0: T.any(String, Symbol),
  )
  .returns(String)
  def self.escape(arg0); end

  Sorbet.sig.returns(MatchData)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(String)
  def self.last_match(arg0=_); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Regexp))
  def self.try_convert(obj); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  Sorbet.sig(
      str: String,
  )
  .returns(T.nilable(Integer))
  def =~(str); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def casefold?(); end

  Sorbet.sig.returns(Encoding)
  def encoding(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def fixed_encoding?(); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig(
      arg0: String,
      options: BasicObject,
      kcode: String,
  )
  .returns(Object)
  Sorbet.sig(
      arg0: Regexp,
  )
  .void
  def initialize(arg0, options=_, kcode=_); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig(
      arg0: T.nilable(String),
      arg1: Integer,
  )
  .returns(T.nilable(MatchData))
  def match(arg0, arg1=_); end

  Sorbet.sig.returns(T::Hash[String, T::Array[Integer]])
  def named_captures(); end

  Sorbet.sig.returns(T::Array[String])
  def names(); end

  Sorbet.sig.returns(Integer)
  def options(); end

  Sorbet.sig.returns(String)
  def source(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def ~(); end

  Sorbet.sig(
      arg0: String,
      options: BasicObject,
      kcode: String,
  )
  .returns(T.self_type)
  Sorbet.sig(
      arg0: Regexp,
  )
  .returns(T.self_type)
  def self.compile(arg0, options=_, kcode=_); end

  Sorbet.sig(
      arg0: T.any(String, Symbol),
  )
  .returns(String)
  def self.quote(arg0); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end
end

# typed: true
class Regexp < Object
  EXTENDED = T.let(T.unsafe(nil), Integer)
  FIXEDENCODING = T.let(T.unsafe(nil), Integer)
  IGNORECASE = T.let(T.unsafe(nil), Integer)
  MULTILINE = T.let(T.unsafe(nil), Integer)
  NOENCODING = T.let(T.unsafe(nil), Integer)

  sig(
      arg0: T.any(String, Symbol),
  )
  .returns(String)
  def self.escape(arg0); end

  sig.returns(MatchData)
  sig(
      arg0: Integer,
  )
  .returns(String)
  def self.last_match(arg0=_); end

  sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Regexp))
  def self.try_convert(obj); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(other); end

  sig(
      str: String,
  )
  .returns(T.nilable(Integer))
  def =~(str); end

  sig.returns(T.any(TrueClass, FalseClass))
  def casefold?(); end

  sig.returns(Encoding)
  def encoding(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def fixed_encoding?(); end

  sig.returns(Integer)
  def hash(); end

  sig(
      arg0: String,
      options: BasicObject,
      kcode: String,
  )
  .returns(Object)
  sig(
      arg0: Regexp,
  )
  .returns(Object)
  def initialize(arg0, options=_, kcode=_); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: String,
      arg1: Integer,
  )
  .returns(T.nilable(MatchData))
  def match(arg0, arg1=_); end

  sig.returns(T::Hash[String, T::Array[Integer]])
  def named_captures(); end

  sig.returns(T::Array[String])
  def names(); end

  sig.returns(Integer)
  def options(); end

  sig.returns(String)
  def source(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(T.nilable(Integer))
  def ~(); end

  sig(
      arg0: String,
      options: BasicObject,
      kcode: String,
  )
  .returns(Regexp)
  sig(
      arg0: Regexp,
  )
  .returns(Regexp)
  def self.compile(arg0, options=_, kcode=_); end

  sig(
      arg0: T.any(String, Symbol),
  )
  .returns(String)
  def self.quote(arg0); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end
end

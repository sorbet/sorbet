# typed: true
class MatchData < Object
  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  Sorbet.sig(
      i_or_start_or_range_or_name: Integer,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      i_or_start_or_range_or_name: Integer,
      length: Integer,
  )
  .returns(T::Array[String])
  Sorbet.sig(
      i_or_start_or_range_or_name: T::Range[Integer],
  )
  .returns(T::Array[String])
  Sorbet.sig(
      i_or_start_or_range_or_name: T.any(String, Symbol),
  )
  .returns(T.nilable(String))
  def [](i_or_start_or_range_or_name, length=T.unsafe(nil)); end

  Sorbet.sig(
      n: Integer,
  )
  .returns(Integer)
  def begin(n); end

  Sorbet.sig.returns(T::Array[String])
  def captures(); end

  Sorbet.sig(
      n: Integer,
  )
  .returns(Integer)
  def end(n); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Integer)
  def length(); end

  Sorbet.sig.returns(T::Array[String])
  def names(); end

  Sorbet.sig(
      n: Integer,
  )
  .returns(T::Array[Integer])
  def offset(n); end

  Sorbet.sig.returns(String)
  def post_match(); end

  Sorbet.sig.returns(String)
  def pre_match(); end

  Sorbet.sig.returns(Regexp)
  def regexp(); end

  Sorbet.sig.returns(Integer)
  def size(); end

  Sorbet.sig.returns(String)
  def string(); end

  Sorbet.sig.returns(T::Array[String])
  def to_a(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig(
      indexes: Integer,
  )
  .returns(T::Array[String])
  def values_at(*indexes); end
end

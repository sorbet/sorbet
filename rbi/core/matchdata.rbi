# typed: true
class MatchData < Object
  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      i_or_start_or_range_or_name: Integer,
  )
  .returns(T.nilable(String))
  sig(
      i_or_start_or_range_or_name: Integer,
      length: Integer,
  )
  .returns(T::Array[String])
  sig(
      i_or_start_or_range_or_name: T::Range[Integer],
  )
  .returns(T::Array[String])
  sig(
      i_or_start_or_range_or_name: T.any(String, Symbol),
  )
  .returns(T.nilable(String))
  def [](i_or_start_or_range_or_name, length=_); end

  sig(
      n: Integer,
  )
  .returns(Integer)
  def begin(n); end

  sig.returns(T::Array[String])
  def captures(); end

  sig(
      n: Integer,
  )
  .returns(Integer)
  def end(n); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(other); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Integer)
  def length(); end

  sig.returns(T::Array[String])
  def names(); end

  sig(
      n: Integer,
  )
  .returns(T::Array[Integer])
  def offset(n); end

  sig.returns(String)
  def post_match(); end

  sig.returns(String)
  def pre_match(); end

  sig.returns(Regexp)
  def regexp(); end

  sig.returns(Integer)
  def size(); end

  sig.returns(String)
  def string(); end

  sig.returns(T::Array[String])
  def to_a(); end

  sig.returns(String)
  def to_s(); end

  sig(
      indexes: Integer,
  )
  .returns(T::Array[String])
  def values_at(*indexes); end
end

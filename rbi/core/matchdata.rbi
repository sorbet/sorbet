# typed: true
class MatchData < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(arg0); end

  sig do
    params(
        i_or_start_or_range_or_name: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        i_or_start_or_range_or_name: Integer,
        length: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        i_or_start_or_range_or_name: T::Range[Integer],
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        i_or_start_or_range_or_name: T.any(String, Symbol),
    )
    .returns(T.nilable(String))
  end
  def [](i_or_start_or_range_or_name, length=T.unsafe(nil)); end

  sig do
    params(
        n: Integer,
    )
    .returns(Integer)
  end
  def begin(n); end

  sig {returns(T::Array[String])}
  def captures(); end

  sig do
    params(
        n: Integer,
    )
    .returns(Integer)
  end
  def end(n); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def eql?(other); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Integer)}
  def length(); end

  sig {returns(T::Array[String])}
  def names(); end

  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Integer])
  end
  def offset(n); end

  sig {returns(String)}
  def post_match(); end

  sig {returns(String)}
  def pre_match(); end

  sig {returns(Regexp)}
  def regexp(); end

  sig {returns(Integer)}
  def size(); end

  sig {returns(String)}
  def string(); end

  sig {returns(T::Array[String])}
  def to_a(); end

  sig {returns(String)}
  def to_s(); end

  sig do
    params(
        indexes: Integer,
    )
    .returns(T::Array[String])
  end
  def values_at(*indexes); end
end

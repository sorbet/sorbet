# typed: true
class Symbol < Object
  include Comparable

  sig {returns(T::Array[Symbol])}
  def self.all_symbols(); end

  sig do
    params(
        other: Symbol,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(obj); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  def =~(obj); end

  sig do
    params(
        idx_or_range: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: Integer,
        n: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: T::Range[Integer],
    )
    .returns(String)
  end
  def [](idx_or_range, n=T.unsafe(nil)); end

  sig {returns(Symbol)}
  def capitalize(); end

  sig do
    params(
        other: Symbol,
    )
    .returns(T.nilable(Integer))
  end
  def casecmp(other); end

  sig {returns(Symbol)}
  def downcase(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def empty?(); end

  sig {returns(Encoding)}
  def encoding(); end

  sig {returns(String)}
  def id2name(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(T.self_type)}
  def intern(); end

  sig {returns(Integer)}
  def length(); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  def match(obj); end

  sig {returns(Symbol)}
  def succ(); end

  sig {returns(Symbol)}
  def swapcase(); end

  sig {returns(Proc)}
  def to_proc(); end

  sig {returns(Symbol)}
  def upcase(); end

  sig {returns(Integer)}
  def size(); end

  sig do
    params(
        idx_or_range: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: Integer,
        n: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: T::Range[Integer],
    )
    .returns(String)
  end
  def slice(idx_or_range, n=T.unsafe(nil)); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(T.self_type)}
  def to_sym(); end
end

# typed: true
class Symbol < Object
  include Comparable

  sig.returns(T::Array[Symbol])
  def self.all_symbols(); end

  sig(
      other: Symbol,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Integer))
  def =~(obj); end

  sig(
      idx_or_range: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: Integer,
      n: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: T::Range[Integer],
  )
  .returns(String)
  def [](idx_or_range, n=_); end

  sig.returns(Symbol)
  def capitalize(); end

  sig(
      other: Symbol,
  )
  .returns(T.nilable(Integer))
  def casecmp(other); end

  sig.returns(Symbol)
  def downcase(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig.returns(Encoding)
  def encoding(); end

  sig.returns(String)
  def id2name(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Symbol)
  def intern(); end

  sig.returns(Integer)
  def length(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Integer))
  def match(obj); end

  sig.returns(Symbol)
  def succ(); end

  sig.returns(Symbol)
  def swapcase(); end

  sig.returns(Proc)
  def to_proc(); end

  sig.returns(Symbol)
  def upcase(); end

  sig.returns(Integer)
  def size(); end

  sig(
      idx_or_range: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: Integer,
      n: Integer,
  )
  .returns(String)
  sig(
      idx_or_range: T::Range[Integer],
  )
  .returns(String)
  def slice(idx_or_range, n=_); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Symbol)
  def to_sym(); end
end

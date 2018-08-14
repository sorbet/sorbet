# typed: true
class Symbol < Object
  include Comparable

  Sorbet.sig.returns(T::Array[Symbol])
  def self.all_symbols(); end

  Sorbet.sig(
      other: Symbol,
  )
  .returns(T.nilable(Integer))
  def <=>(other); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(obj); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Integer))
  def =~(obj); end

  Sorbet.sig(
      idx_or_range: Integer,
  )
  .returns(String)
  Sorbet.sig(
      idx_or_range: Integer,
      n: Integer,
  )
  .returns(String)
  Sorbet.sig(
      idx_or_range: T::Range[Integer],
  )
  .returns(String)
  def [](idx_or_range, n=_); end

  Sorbet.sig.returns(Symbol)
  def capitalize(); end

  Sorbet.sig(
      other: Symbol,
  )
  .returns(T.nilable(Integer))
  def casecmp(other); end

  Sorbet.sig.returns(Symbol)
  def downcase(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  Sorbet.sig.returns(Encoding)
  def encoding(); end

  Sorbet.sig.returns(String)
  def id2name(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(T.self_type)
  def intern(); end

  Sorbet.sig.returns(Integer)
  def length(); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.nilable(Integer))
  def match(obj); end

  Sorbet.sig.returns(Symbol)
  def succ(); end

  Sorbet.sig.returns(Symbol)
  def swapcase(); end

  Sorbet.sig.returns(Proc)
  def to_proc(); end

  Sorbet.sig.returns(Symbol)
  def upcase(); end

  Sorbet.sig.returns(Integer)
  def size(); end

  Sorbet.sig(
      idx_or_range: Integer,
  )
  .returns(String)
  Sorbet.sig(
      idx_or_range: Integer,
      n: Integer,
  )
  .returns(String)
  Sorbet.sig(
      idx_or_range: T::Range[Integer],
  )
  .returns(String)
  def slice(idx_or_range, n=_); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(T.self_type)
  def to_sym(); end
end

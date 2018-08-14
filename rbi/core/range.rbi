# typed: true
class Range < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(obj); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(obj); end

  Sorbet.sig.returns(Elem)
  def begin(); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.any(TrueClass, FalseClass)),
  )
  .returns(T.nilable(T.type_parameter(:U)))
  def bsearch(&blk); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def cover?(obj); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[Elem])
  def each(&blk); end

  Sorbet.sig.returns(Elem)
  def end(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def exclude_end?(); end

  Sorbet.sig.returns(Elem)
  Sorbet.sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def first(n=_); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(obj); end

  Sorbet.sig(
      _begin: Elem,
      _end: Elem,
      exclude_end: T.any(TrueClass, FalseClass),
  )
  .void
  def initialize(_begin, _end, exclude_end=_); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Elem)
  Sorbet.sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def last(n=_); end

  Sorbet.sig.returns(Elem)
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  Sorbet.sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  Sorbet.sig(
      n: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max(n=_, &blk); end

  Sorbet.sig.returns(Elem)
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  Sorbet.sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  Sorbet.sig(
      n: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min(n=_, &blk); end

  Sorbet.sig.returns(T.nilable(Integer))
  def size(); end

  Sorbet.sig(
      n: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def step(n=_, &blk); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(obj); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(obj); end
end

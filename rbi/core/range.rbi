# typed: true
class Range < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(obj); end

  sig.returns(Elem)
  def begin(); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.any(TrueClass, FalseClass)),
  )
  .returns(T.nilable(T.type_parameter(:U)))
  def bsearch(&blk); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def cover?(obj); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  sig.returns(Enumerator[Elem])
  def each(&blk); end

  sig.returns(Elem)
  def end(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def exclude_end?(); end

  sig.returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def first(n=_); end

  sig.returns(Integer)
  def hash(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(obj); end

  sig(
      _begin: Elem,
      _end: Elem,
      exclude_end: T.any(TrueClass, FalseClass),
  )
  .returns(Object)
  def initialize(_begin, _end, exclude_end=_); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def last(n=_); end

  sig.returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      n: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max(n=_, &blk); end

  sig.returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Elem)
  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      n: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min(n=_, &blk); end

  sig.returns(T.nilable(Integer))
  def size(); end

  sig(
      n: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def step(n=_, &blk); end

  sig.returns(String)
  def to_s(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(obj); end
end

# typed: true
class Enumerator < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(T.self_type)
  def each(&blk); end

  sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[[Elem, Integer]])
  def each_with_index(&blk); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
      blk: T.proc(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject),
  )
  .returns(T.untyped)
  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(Enumerator[[Elem, T.type_parameter(:U)]])
  def each_with_object(arg0, &blk); end

  sig(
      arg0: Elem,
  )
  .returns(NilClass)
  def feed(arg0); end

  type_parameters(:U).sig(
      arg0: Integer,
      blk: T.proc(arg0: T::Array[T.type_parameter(:U)]).returns(BasicObject),
  )
  .returns(Object)
  type_parameters(:U).sig(
      arg0: Proc,
      blk: T.proc(arg0: T::Array[T.type_parameter(:U)]).returns(BasicObject),
  )
  .void
  def initialize(arg0=_, &blk); end

  sig.returns(String)
  def inspect(); end

  sig.returns(Elem)
  def next(); end

  sig.returns(T::Array[Elem])
  def next_values(); end

  sig.returns(Elem)
  def peek(); end

  sig.returns(T::Array[Elem])
  def peek_values(); end

  sig.returns(T.self_type)
  def rewind(); end

  sig.returns(T.nilable(T.any(Integer, Float)))
  def size(); end

  sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T.untyped)
  sig.returns(Enumerator[[Elem, Integer]])
  def with_index(&blk); end

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
      blk: T.proc(arg0: Elem, arg1: T.type_parameter(:U)).returns(BasicObject),
  )
  .returns(T.untyped)
  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(Enumerator[[Elem, T.type_parameter(:U)]])
  def with_object(arg0, &blk); end
end

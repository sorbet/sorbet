# typed: true
class Enumerator < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.untyped)
  Sorbet.sig.returns(T.self_type)
  def each(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T.untyped)
  Sorbet.sig.returns(Enumerator[[Elem, Integer]])
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

  Sorbet.sig(
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

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Elem)
  def next(); end

  Sorbet.sig.returns(T::Array[Elem])
  def next_values(); end

  Sorbet.sig.returns(Elem)
  def peek(); end

  Sorbet.sig.returns(T::Array[Elem])
  def peek_values(); end

  Sorbet.sig.returns(T.self_type)
  def rewind(); end

  Sorbet.sig.returns(T.nilable(T.any(Integer, Float)))
  def size(); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T.untyped)
  Sorbet.sig.returns(Enumerator[[Elem, Integer]])
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

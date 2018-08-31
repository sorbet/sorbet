# typed: true
module Enumerable
  extend T::Generic
  Elem = type_member(:out)

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def all?(&blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def any?(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  Sorbet.sig.returns(Enumerator[Elem])
  def collect(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(Enumerator[T.type_parameter(:U)]),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def collect_concat(&blk); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(Integer)
  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Integer)
  def count(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig(
      n: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(NilClass)
  Sorbet.sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def cycle(n=T.unsafe(nil), &blk); end

  Sorbet.sig(
      ifnone: Proc,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Elem))
  Sorbet.sig(
      ifnone: Proc,
  )
  .returns(Enumerator[Elem])
  def detect(ifnone=T.unsafe(nil), &blk); end

  Sorbet.sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def drop(n); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def drop_while(&blk); end

  Sorbet.sig(
      n: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(NilClass)
  Sorbet.sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def each_cons(n, &blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T::Enumerable[Elem])
  Sorbet.sig.returns(Enumerator[[Elem, Integer]])
  def each_with_index(&blk); end

  Sorbet.sig.returns(T::Array[Elem])
  def entries(); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def find_all(&blk); end

  Sorbet.sig(
      value: BasicObject,
  )
  .returns(T.nilable(Integer))
  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Integer))
  Sorbet.sig.returns(Enumerator[Elem])
  def find_index(value=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(T.nilable(Elem))
  Sorbet.sig(
      n: Integer,
  )
  .returns(T.nilable(T::Array[Elem]))
  def first(n=T.unsafe(nil)); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T::Array[Elem])
  type_parameters(:U).sig(
      arg0: BasicObject,
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def grep(arg0, &blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Hash[T.type_parameter(:U), T::Array[Elem]])
  Sorbet.sig.returns(Enumerator[Elem])
  def group_by(&blk); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  type_parameters(:Any).sig(
      initial: T.type_parameter(:Any),
      arg0: Symbol,
  )
  .returns(T.untyped)
  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  Sorbet.sig(
      initial: Elem,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(T.nilable(Elem))
  def inject(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(T.nilable(Elem))
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  Sorbet.sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(Enumerator[Elem])
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  Sorbet.sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max_by(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(T.nilable(Elem))
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  Sorbet.sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(Enumerator[Elem])
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  Sorbet.sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min_by(arg0=T.unsafe(nil), &blk); end

  Sorbet.sig.returns([T.nilable(Elem), T.nilable(Elem)])
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns([T.nilable(Elem), T.nilable(Elem)])
  def minmax(&blk); end

  Sorbet.sig.returns([T.nilable(Elem), T.nilable(Elem)])
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Enumerator[Elem])
  def minmax_by(&blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def none?(&blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def one?(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns([T::Array[Elem], T::Array[Elem]])
  Sorbet.sig.returns(Enumerator[Elem])
  def partition(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def reject(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Enumerator[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def reverse_each(&blk); end

  Sorbet.sig.returns(T::Array[Elem])
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def sort(&blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def sort_by(&blk); end

  Sorbet.sig(
      n: Integer,
  )
  .returns(T.nilable(T::Array[Elem]))
  def take(n); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def take_while(&blk); end

  Sorbet.sig.returns(T::Hash[Elem, Elem])
  def to_h(); end

  Sorbet.sig(
      n: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(NilClass)
  Sorbet.sig(
      n: Integer,
  )
  .returns(Enumerator[T::Array[Elem]])
  def each_slice(n, &blk); end

  Sorbet.sig(
      ifnone: Proc,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Elem))
  Sorbet.sig(
      ifnone: Proc,
  )
  .returns(Enumerator[Elem])
  def find(ifnone=T.unsafe(nil), &blk); end

  # N.B. this signature is wrong; Our generic method implementation
  # cannot model the correct signature, so we pass through the return
  # type of the block and then fix it up in an ad-hoc way in Ruby. A
  # more-correct signature might be:
  #   type_parameters(:U).sig(
  #       blk: T.proc(arg0: Elem).returns(T.any(T::Array[T.type_parameter(:U)], T.type_parameter(:U)),
  #   )
  #   .returns(T.type_parameter(:U))
  #
  # But that would require a lot more sophistication from our generic
  # method inference.
  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T.type_parameter(:U))
  Sorbet.sig.returns(Enumerator[Elem])
  def flat_map(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  Sorbet.sig.returns(Enumerator[Elem])
  def map(&blk); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(arg0); end

  type_parameters(:Any).sig(
      initial: T.type_parameter(:Any),
      arg0: Symbol,
  )
  .returns(T.untyped)
  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  Sorbet.sig(
      initial: Elem,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  Sorbet.sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(T.nilable(Elem))
  def reduce(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  Sorbet.sig.returns(Enumerator[Elem])
  def select(&blk); end

  Sorbet.sig.returns(T::Array[Elem])
  def to_a(); end
end

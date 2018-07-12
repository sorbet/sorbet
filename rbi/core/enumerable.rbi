# typed: true
module Enumerable
  extend T::Generic
  Elem = type_member(:out)

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def all?(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def any?(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def collect(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(Enumerator[T.type_parameter(:U)]),
  )
  .returns(T::Array[T.type_parameter(:U)])
  def collect_concat(&blk); end

  sig.returns(Integer)
  sig(
      arg0: BasicObject,
  )
  .returns(Integer)
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Integer)
  def count(arg0=_, &blk); end

  sig(
      n: Integer,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def cycle(n=_, &blk); end

  sig(
      ifnone: Proc,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Elem))
  sig(
      ifnone: Proc,
  )
  .returns(Enumerator[Elem])
  def detect(ifnone=_, &blk); end

  sig(
      n: Integer,
  )
  .returns(T::Array[Elem])
  def drop(n); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def drop_while(&blk); end

  sig(
      n: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      n: Integer,
  )
  .returns(Enumerator[Elem])
  def each_cons(n, &blk); end

  sig(
      blk: T.proc(arg0: Elem, arg1: Integer).returns(BasicObject),
  )
  .returns(T::Enumerable[Elem])
  sig.returns(Enumerator[[Elem, Integer]])
  def each_with_index(&blk); end

  sig.returns(T::Array[Elem])
  def entries(); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def find_all(&blk); end

  sig(
      value: BasicObject,
  )
  .returns(T.nilable(Integer))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Integer))
  sig.returns(Enumerator[Elem])
  def find_index(value=_, &blk); end

  sig.returns(T.nilable(Elem))
  sig(
      n: Integer,
  )
  .returns(T.nilable(T::Array[Elem]))
  def first(n=_); end

  sig(
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
  sig.returns(Enumerator[Elem])
  def group_by(&blk); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(arg0); end

  type_parameters(:Any).sig(
      initial: T.type_parameter(:Any),
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      initial: Elem,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(T.nilable(Elem))
  def inject(initial=_, arg0=_, &blk); end

  sig.returns(T.nilable(Elem))
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max(arg0=_, &blk); end

  sig.returns(Enumerator[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def max_by(arg0=_, &blk); end

  sig.returns(T.nilable(Elem))
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  sig(
      arg0: Integer,
  )
  .returns(T::Array[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min(arg0=_, &blk); end

  sig.returns(Enumerator[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T.nilable(Elem))
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Elem])
  sig(
      arg0: Integer,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def min_by(arg0=_, &blk); end

  sig.returns([T.nilable(Elem), T.nilable(Elem)])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns([T.nilable(Elem), T.nilable(Elem)])
  def minmax(&blk); end

  sig.returns([T.nilable(Elem), T.nilable(Elem)])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(Enumerator[Elem])
  def minmax_by(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def none?(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.any(TrueClass, FalseClass))
  def one?(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns([T::Array[Elem], T::Array[Elem]])
  sig.returns(Enumerator[Elem])
  def partition(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def reject(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(Enumerator[Elem])
  sig.returns(Enumerator[Elem])
  def reverse_each(&blk); end

  sig.returns(T::Array[Elem])
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Integer),
  )
  .returns(T::Array[Elem])
  def sort(&blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def sort_by(&blk); end

  sig(
      n: Integer,
  )
  .returns(T.nilable(T::Array[Elem]))
  def take(n); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def take_while(&blk); end

  sig.returns(T::Hash[Elem, Elem])
  def to_h(); end

  sig(
      n: Integer,
      blk: T.proc(arg0: T::Array[Elem]).returns(BasicObject),
  )
  .returns(NilClass)
  sig(
      n: Integer,
  )
  .returns(Enumerator[T::Array[Elem]])
  def each_slice(n, &blk); end

  sig(
      ifnone: Proc,
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(Elem))
  sig(
      ifnone: Proc,
  )
  .returns(Enumerator[Elem])
  def find(ifnone=_, &blk); end

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
  sig.returns(Enumerator[Elem])
  def flat_map(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Array[T.type_parameter(:U)])
  sig.returns(Enumerator[Elem])
  def map(&blk); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(arg0); end

  type_parameters(:Any).sig(
      initial: T.type_parameter(:Any),
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      arg0: Symbol,
  )
  .returns(T.untyped)
  sig(
      initial: Elem,
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(Elem)
  sig(
      blk: T.proc(arg0: Elem, arg1: Elem).returns(Elem),
  )
  .returns(T.nilable(Elem))
  def reduce(initial=_, arg0=_, &blk); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Array[Elem])
  sig.returns(Enumerator[Elem])
  def select(&blk); end

  sig.returns(T::Array[Elem])
  def to_a(); end
end

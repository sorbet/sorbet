# typed: true
module Enumerable
  extend T::Generic
  Elem = type_member(:out)

  abstract!

  sig do
    abstract.
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def all?(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def any?(&blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def collect(&blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(Enumerator[T.type_parameter(:U)]),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def collect_concat(&blk); end

  sig {returns(Integer)}
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(Integer)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Integer)
  end
  def count(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(Enumerator[Elem])
  end
  def cycle(n=T.unsafe(nil), &blk); end

  sig do
    params(
        ifnone: Proc,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        ifnone: Proc,
    )
    .returns(Enumerator[Elem])
  end
  def detect(ifnone=T.unsafe(nil), &blk); end

  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def drop(n); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def drop_while(&blk); end

  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(Enumerator[T::Array[Elem]])
  end
  def each_cons(n, &blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Integer).returns(BasicObject),
    )
    .returns(T::Enumerable[Elem])
  end
  sig {returns(Enumerator[[Elem, Integer]])}
  def each_with_index(&blk); end

  sig {returns(T::Array[Elem])}
  def entries(); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def find_all(&blk); end

  sig do
    params(
        value: BasicObject,
    )
    .returns(T.nilable(Integer))
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Integer))
  end
  sig {returns(Enumerator[Elem])}
  def find_index(value=T.unsafe(nil), &blk); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        n: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def first(n=T.unsafe(nil)); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Array[Elem])
  end
  sig do
    type_parameters(:U).params(
        arg0: BasicObject,
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def grep(arg0, &blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Hash[T.type_parameter(:U), T::Array[Elem]])
  end
  sig {returns(Enumerator[Elem])}
  def group_by(&blk); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def include?(arg0); end

  sig do
    type_parameters(:Any).params(
        initial: T.type_parameter(:Any),
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        initial: Elem,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Elem),
    )
    .returns(Elem)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Elem),
    )
    .returns(T.nilable(Elem))
  end
  def inject(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def max(arg0=T.unsafe(nil), &blk); end

  sig {returns(Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(T::Array[Elem])
  end
  def max_by(arg0=T.unsafe(nil), &blk); end

  sig {returns(T.nilable(Elem))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min(arg0=T.unsafe(nil), &blk); end

  sig {returns(Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min_by(arg0=T.unsafe(nil), &blk); end

  sig {returns([T.nilable(Elem), T.nilable(Elem)])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns([T.nilable(Elem), T.nilable(Elem)])
  end
  def minmax(&blk); end

  sig {returns([T.nilable(Elem), T.nilable(Elem)])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(Enumerator[Elem])
  end
  def minmax_by(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def none?(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def one?(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns([T::Array[Elem], T::Array[Elem]])
  end
  sig {returns(Enumerator[Elem])}
  def partition(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def reject(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(Enumerator[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def reverse_each(&blk); end

  sig {returns(T::Array[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def sort(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def sort_by(&blk); end

  sig do
    params(
        n: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def take(n); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def take_while(&blk); end

  sig {returns(T::Hash[Elem, Elem])}
  def to_h(); end

  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: T::Array[Elem]).returns(BasicObject),
    )
    .returns(NilClass)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(Enumerator[T::Array[Elem]])
  end
  def each_slice(n, &blk); end

  sig do
    params(
        ifnone: Proc,
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        ifnone: Proc,
    )
    .returns(Enumerator[Elem])
  end
  def find(ifnone=T.unsafe(nil), &blk); end

  # N.B. this signature is wrong; Our generic method implementation
  # cannot model the correct signature, so we pass through the return
  # type of the block and then fix it up in an ad-hoc way in Ruby. A
  # more-correct signature might be:
  #   sig do
  #     type_parameters(:U).params(
  #       blk: T.proc.params(arg0: Elem).returns(T.any(T::Array[T.type_parameter(:U)], T.type_parameter(:U)),
  #     )
  #     .returns(T.type_parameter(:U))
  #   end
  #
  # But that would require a lot more sophistication from our generic
  # method inference.
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  sig {returns(Enumerator[Elem])}
  def flat_map(&blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def map(&blk); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def member?(arg0); end

  sig do
    type_parameters(:Any).params(
        initial: T.type_parameter(:Any),
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        initial: Elem,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Elem),
    )
    .returns(Elem)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Elem),
    )
    .returns(T.nilable(Elem))
  end
  def reduce(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def select(&blk); end

  sig {returns(T::Array[Elem])}
  def to_a(); end
end

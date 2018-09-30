# typed: true
class Set < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  sig do
    type_parameters(:U).params(
        ary: T.type_parameter(:U),
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def self.[](*ary); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def +(enum); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def ^(enum); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def add(o); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.nilable(T.self_type))
  end
  def add?(o); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: T.type_parameter(:U)).returns(Elem),
    )
    .returns(T::Hash[T.type_parameter(:U), T::Set[Elem]])
  end
  def classify(&blk); end

  sig {returns(T.self_type)}
  def clear(); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def delete(o); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.nilable(T.self_type))
  end
  def delete?(o); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  def delete_if(&blk); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def difference(enum); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def disjoint?(set); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[Elem])}
  def each(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def empty?(); end

  sig {returns(Set)}
  def flatten(); end

  sig {returns(T.nilable(T.self_type))}
  def flatten!(); end

  sig do
    type_parameters(:U).params(
        enum: T::Enumerable[T.type_parameter(:U)],
    )
    .void
  end
  def initialize(enum=T.unsafe(nil)); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def intersect?(set); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def intersection(enum); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  def keep_if(&blk); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def map!(&blk); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def member?(o); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T.self_type)
  end
  def merge(enum); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def proper_subset?(set); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def proper_superset?(set); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(T.self_type))
  end
  def reject!(&blk); end

  sig do
    type_parameters(:U).params(
        enum: T::Enumerable[T.type_parameter(:U)],
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def replace(enum); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.nilable(T.self_type))
  end
  def select!(&blk); end

  sig {returns(Integer)}
  def size(); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def subset?(set); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T.self_type)
  end
  def subtract(enum); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def superset?(set); end

  sig {returns(T::Array[Elem])}
  def to_a(); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def &(enum); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def -(enum); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def <(set); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.self_type)
  end
  def <<(o); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def <=(set); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def >(set); end

  sig do
    params(
        set: T::Set[Elem],
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def >=(set); end

  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Set[T.type_parameter(:U)])
  end
  def collect!(&blk); end

  sig do
    params(
        o: Elem,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def include?(o); end

  sig {returns(Integer)}
  def length(); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def |(enum); end

  sig do
    params(
        enum: T::Enumerable[Elem],
    )
    .returns(T::Set[Elem])
  end
  def union(enum); end
end

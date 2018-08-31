# typed: true
class Set < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)

  type_parameters(:U).sig(
      ary: T.type_parameter(:U),
  )
  .returns(T::Set[T.type_parameter(:U)])
  def self.[](*ary); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def +(enum); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def ^(enum); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.self_type)
  def add(o); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.nilable(T.self_type))
  def add?(o); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: T.type_parameter(:U)).returns(Elem),
  )
  .returns(T::Hash[T.type_parameter(:U), T::Set[Elem]])
  def classify(&blk); end

  Sorbet.sig.returns(T.self_type)
  def clear(); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.self_type)
  def delete(o); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.nilable(T.self_type))
  def delete?(o); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  def delete_if(&blk); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def difference(enum); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def disjoint?(set); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  Sorbet.sig.returns(Enumerator[Elem])
  def each(&blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  Sorbet.sig.returns(Set)
  def flatten(); end

  Sorbet.sig.returns(T.nilable(T.self_type))
  def flatten!(); end

  type_parameters(:U).sig(
      enum: T::Enumerable[T.type_parameter(:U)],
  )
  .void
  def initialize(enum=T.unsafe(nil)); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def intersect?(set); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def intersection(enum); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.self_type)
  def keep_if(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Set[T.type_parameter(:U)])
  def map!(&blk); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(o); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T.self_type)
  def merge(enum); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def proper_subset?(set); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def proper_superset?(set); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(T.self_type))
  def reject!(&blk); end

  type_parameters(:U).sig(
      enum: T::Enumerable[T.type_parameter(:U)],
  )
  .returns(T::Set[T.type_parameter(:U)])
  def replace(enum); end

  Sorbet.sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(T.self_type))
  def select!(&blk); end

  Sorbet.sig.returns(Integer)
  def size(); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def subset?(set); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T.self_type)
  def subtract(enum); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def superset?(set); end

  Sorbet.sig.returns(T::Array[Elem])
  def to_a(); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def &(enum); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def -(enum); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(set); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.self_type)
  def <<(o); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(set); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(set); end

  Sorbet.sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(set); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Set[T.type_parameter(:U)])
  def collect!(&blk); end

  Sorbet.sig(
      o: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(o); end

  Sorbet.sig.returns(Integer)
  def length(); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def |(enum); end

  Sorbet.sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def union(enum); end
end

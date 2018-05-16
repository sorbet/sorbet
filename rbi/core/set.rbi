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

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def +(enum); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def ^(enum); end

  sig(
      o: Elem,
  )
  .returns(T::Set[Elem])
  def add(o); end

  type_parameters(:Self).sig(
      o: Elem,
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def add?(o); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: T.type_parameter(:U)).returns(Elem),
  )
  .returns(T::Hash[T.type_parameter(:U), T::Set[Elem]])
  def classify(&blk); end

  sig.returns(T::Set[Elem])
  def clear(); end

  sig(
      o: Elem,
  )
  .returns(T::Set[Elem])
  def delete(o); end

  type_parameters(:Self).sig(
      o: Elem,
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def delete?(o); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Set[Elem])
  def delete_if(&blk); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def difference(enum); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def disjoint?(set); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Set[Elem])
  sig.returns(Enumerator[Elem])
  def each(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig.returns(Set)
  def flatten(); end

  type_parameters(:Self).sig.returns(T.nilable(T.type_parameter(:Self)))
  def flatten!(); end

  type_parameters(:U).sig(
      enum: T::Enumerable[T.type_parameter(:U)],
  )
  .returns(Object)
  def initialize(enum=_); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def intersect?(set); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def intersection(enum); end

  sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T::Set[Elem])
  def keep_if(&blk); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Set[T.type_parameter(:U)])
  def map!(&blk); end

  sig(
      o: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(o); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def merge(enum); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def proper_subset?(set); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def proper_superset?(set); end

  type_parameters(:Self).sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def reject!(&blk); end

  type_parameters(:U).sig(
      enum: T::Enumerable[T.type_parameter(:U)],
  )
  .returns(T::Set[T.type_parameter(:U)])
  def replace(enum); end

  type_parameters(:Self).sig(
      blk: T.proc(arg0: Elem).returns(BasicObject),
  )
  .returns(T.nilable(T.type_parameter(:Self)))
  def select!(&blk); end

  sig.returns(Integer)
  def size(); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def subset?(set); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def subtract(enum); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def superset?(set); end

  sig.returns(T::Array[Elem])
  def to_a(); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def &(enum); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def -(enum); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(set); end

  sig(
      o: Elem,
  )
  .returns(T::Set[Elem])
  def <<(o); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(set); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(set); end

  sig(
      set: T::Set[Elem],
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(set); end

  type_parameters(:U).sig(
      blk: T.proc(arg0: Elem).returns(T.type_parameter(:U)),
  )
  .returns(T::Set[T.type_parameter(:U)])
  def collect!(&blk); end

  sig(
      o: Elem,
  )
  .returns(T.any(TrueClass, FalseClass))
  def include?(o); end

  sig.returns(Integer)
  def length(); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def |(enum); end

  sig(
      enum: T::Enumerable[Elem],
  )
  .returns(T::Set[Elem])
  def union(enum); end
end

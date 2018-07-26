# typed: true
class Hash < Object
  include Enumerable

  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)

  type_parameters(:U).sig(
      arg0: T.type_parameter(:U),
  )
  .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:U)])
  def self.[](*arg0); end

  sig(
      arg0: K,
  )
  .returns(T.nilable(V))
  def [](arg0); end

  sig(
      arg0: K,
      arg1: V,
  )
  .returns(V)
  def []=(arg0, arg1); end

  sig(
      arg0: K,
  )
  .returns(T::Array[T.any(K, V)])
  def assoc(arg0); end

  sig.returns(T::Hash[K, V])
  def clear(); end

  sig.returns(T::Hash[K, V])
  def compare_by_identity(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def compare_by_identity?(); end

  sig(
      arg0: K,
  )
  .returns(T.nilable(V))
  sig(
      arg0: K,
      blk: T.proc(arg0: K).returns(V),
  )
  .returns(T.nilable(V))
  def default(arg0=_, &blk); end

  sig(
      arg0: V,
  )
  .returns(V)
  def default=(arg0); end

  sig(
      arg0: K,
  )
  .returns(T.nilable(V))
  type_parameters(:U).sig(
      arg0: K,
      blk: T.proc(arg0: K).returns(T.type_parameter(:U)),
  )
  .returns(T.any(T.type_parameter(:U), V))
  def delete(arg0, &blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def delete_if(&blk); end

  sig(
      blk: T.proc(arg0: [K, V]).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each(&blk); end

  sig(
      blk: T.proc(arg0: K).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each_key(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each_pair(&blk); end

  sig(
      blk: T.proc(arg0: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def each_value(&blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def empty?(); end

  sig(
      arg0: K,
  )
  .returns(V)
  sig(
      arg0: K,
      arg1: V,
  )
  .returns(V)
  sig(
      arg0: K,
      blk: T.proc(arg0: K).returns(V),
  )
  .returns(V)
  def fetch(arg0, arg1=_, &blk); end

  sig(
      arg0: K,
  )
  .returns(T.any(TrueClass, FalseClass))
  def has_key?(arg0); end

  sig(
      arg0: V,
  )
  .returns(T.any(TrueClass, FalseClass))
  def has_value?(arg0); end

  sig.returns(Hash)
  sig(
      default: Object,
  )
  .returns(Hash)
  sig(
      blk: T.proc(hash: Hash, key: Object).returns(Object)
  )
  .void
  def initialize(default=_, &blk); end

  sig.returns(String)
  def inspect(); end

  sig.returns(T::Hash[V, K])
  def invert(); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  sig.returns(Enumerator[[K, V]])
  def keep_if(&blk); end

  sig(
      arg0: V,
  )
  .returns(T.nilable(K))
  def key(arg0); end

  sig(
      arg0: K,
  )
  .returns(T.any(TrueClass, FalseClass))
  def key?(arg0); end

  sig.returns(T::Array[K])
  def keys(); end

  sig.returns(Integer)
  def length(); end

  sig(
      arg0: K,
  )
  .returns(T.any(TrueClass, FalseClass))
  def member?(arg0); end

  type_parameters(:A ,:B).sig(
      arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
  )
  .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  type_parameters(:A ,:B).sig(
      arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
      blk: T.proc(arg0: K, arg1: V, arg2: T.type_parameter(:B)).returns(T.any(V, T.type_parameter(:B))),
  )
  .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  def merge(arg0, &blk); end

  sig(
      arg0: K,
  )
  .returns(T::Array[T.any(K, V)])
  def rassoc(arg0); end

  sig.returns(T::Hash[K, V])
  def rehash(); end

  sig.returns(Enumerator[[K, V]])
  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def reject(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def reject!(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def select(&blk); end

  sig(
      blk: T.proc(arg0: K, arg1: V).returns(BasicObject),
  )
  .returns(T::Hash[K, V])
  def select!(&blk); end

  sig.returns(T::Array[T.any(K, V)])
  def shift(); end

  sig.returns(Integer)
  def size(); end

  sig(
      arg0: K,
      arg1: V,
  )
  .returns(V)
  def store(arg0, arg1); end

  sig.returns(T::Array[T::Array[T.any(K, V)]])
  def to_a(); end

  sig.returns(T::Hash[K, V])
  def to_hash(); end

  sig.returns(String)
  def to_s(); end

  sig(
      arg0: V,
  )
  .returns(T.any(TrueClass, FalseClass))
  def value?(arg0); end

  sig.returns(T::Array[V])
  def values(); end

  sig(
      arg0: K,
  )
  .returns(T::Array[V])
  def values_at(*arg0); end
end

# typed: true
class Hash < Object
  include Enumerable

  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)

  sig do
    type_parameters(:U, :V).params(
      arg0: T::Array[[T.type_parameter(:U), T.type_parameter(:V)]],
    )
    .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:V)])
  end
  def self.[](*arg0); end

  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  def [](arg0); end

  sig do
    params(
        arg0: K,
        arg1: V,
    )
    .returns(V)
  end
  def []=(arg0, arg1); end

  sig do
    params(
        arg0: K,
    )
    .returns(T::Array[T.any(K, V)])
  end
  def assoc(arg0); end

  sig {returns(T::Hash[K, V])}
  def clear(); end

  sig {returns(T::Hash[K, V])}
  def compare_by_identity(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def compare_by_identity?(); end

  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  sig do
    params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(V),
    )
    .returns(T.nilable(V))
  end
  def default(arg0=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: V,
    )
    .returns(V)
  end
  def default=(arg0); end

  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  sig do
    type_parameters(:U).params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(T.type_parameter(:U)),
    )
    .returns(T.any(T.type_parameter(:U), V))
  end
  def delete(arg0, &blk); end

  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def delete_if(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: [K, V]).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def each(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: K).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def each_key(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def each_pair(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def each_value(&blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def empty?(); end

  sig do
    params(
        arg0: K,
    )
    .returns(V)
  end
  sig do
    params(
        arg0: K,
        arg1: V,
    )
    .returns(V)
  end
  sig do
    params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(V),
    )
    .returns(V)
  end
  def fetch(arg0, arg1=T.unsafe(nil), &blk); end

  sig do
    params(
        arg0: K,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def has_key?(arg0); end

  sig do
    params(
        arg0: V,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def has_value?(arg0); end

  sig {returns(Hash)}
  sig do
    params(
        default: Object,
    )
    .returns(Hash)
  end
  sig do
    params(
        blk: T.proc.params(hash: Hash, key: Object).returns(Object)
    )
    .void
  end
  def initialize(default=T.unsafe(nil), &blk); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(T::Hash[V, K])}
  def invert(); end

  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def keep_if(&blk); end

  sig do
    params(
        arg0: V,
    )
    .returns(T.nilable(K))
  end
  def key(arg0); end

  sig do
    params(
        arg0: K,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def key?(arg0); end

  sig {returns(T::Array[K])}
  def keys(); end

  sig {returns(Integer)}
  def length(); end

  sig do
    params(
        arg0: K,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def member?(arg0); end

  sig do
    type_parameters(:A ,:B).params(
        arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  sig do
    type_parameters(:A ,:B).params(
        arg0: T::Hash[T.type_parameter(:A), T.type_parameter(:B)],
        blk: T.proc.params(arg0: K, arg1: V, arg2: T.type_parameter(:B)).returns(T.any(V, T.type_parameter(:B))),
    )
    .returns(T::Hash[T.any(T.type_parameter(:A), K), T.any(T.type_parameter(:B), V)])
  end
  def merge(arg0, &blk); end

  sig do
    params(
        arg0: K,
    )
    .returns(T::Array[T.any(K, V)])
  end
  def rassoc(arg0); end

  sig {returns(T::Hash[K, V])}
  def rehash(); end

  sig {returns(Enumerator[[K, V]])}
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def reject(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def reject!(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def select(&blk); end

  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def select!(&blk); end

  sig {returns(T::Array[T.any(K, V)])}
  def shift(); end

  sig {returns(Integer)}
  def size(); end

  sig do
    params(
        arg0: K,
        arg1: V,
    )
    .returns(V)
  end
  def store(arg0, arg1); end

  sig {returns(T::Array[[K, V]])}
  def to_a(); end

  sig {returns(T::Hash[K, V])}
  def to_hash(); end

  sig {returns(String)}
  def to_s(); end

  sig do
    params(
        arg0: V,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def value?(arg0); end

  sig {returns(T::Array[V])}
  def values(); end

  sig do
    params(
        arg0: K,
    )
    .returns(T::Array[V])
  end
  def values_at(*arg0); end
end

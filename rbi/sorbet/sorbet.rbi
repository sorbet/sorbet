# typed: __STDLIB_INTERNAL
module Sorbet::Private::Static
  sig do
    params(
        arg0: T.untyped,
        arg1: T.nilable(Symbol),
        blk: T.proc.bind(T::Private::Methods::DeclBuilder).void
    )
    .void
  end
  def self.sig(arg0, arg1=nil, &blk)
  end

  sig do
    params(
        expr: T.untyped,
    )
    .void
  end
  def self.keep_for_ide(expr)
  end

  sig do
    params(
        expr: T.untyped,
    )
    .void
  end
  def self.keep_for_typechecking(expr)
  end

  sig do
    type_parameters(:U)
      .params(this: T.untyped, fun: T.all(T.type_parameter(:U), Symbol), kind: Symbol)
      .returns(T.type_parameter(:U))
  end
  def self.keep_def(this, fun, kind)
  end

  sig do
    type_parameters(:U)
      .params(this: T.untyped, fun: T.all(T.type_parameter(:U), Symbol), kind: Symbol)
      .returns(T.type_parameter(:U))
  end
  def self.keep_self_def(this, fun, kind)
  end

  # Used to implement Enumerable#to_h, which forwards here in C++.
  #
  # Implemented this way because we don't have a way to destructure
  # `self` in the type system yet, but generic methods *do* let us
  # destructure arguments
  sig do
    type_parameters(:U, :V).params(
      arg0: T::Enumerable[[T.type_parameter(:U), T.type_parameter(:V)]],
    )
    .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:V)])
  end
  def self.enumerable_to_h(*arg0); end
end

module Sorbet::Private::Static::ResolvedSig
  # This sig is a bit of a fib: `sig_arg` is technically T.nilable(Symbol), so
  # we might have sends to this method that only have three args: `original_recv`,
  # `is_self_method`, and `method_name`.  But we never resolve sends to this
  # method; any sends to this method are resolved instead as sends to
  # original_recv.sig with the last two args dropped.
  sig do
    params(
        original_recv: T.untyped,
        sig_arg: T.untyped,
        is_self_method: T.untyped,
        method_name: T.untyped,
        blk: T.proc.bind(T::Private::Methods::DeclBuilder).void
    )
    .void
  end
  def self.sig(original_recv, sig_arg, is_self_method, method_name=nil, &blk)
  end
end

module Sorbet::Private::Static::StubModule
end

class Sorbet::Private::Static::ImplicitModuleSuperclass < BasicObject
end

# C++ code delegates the implementation of many methods on tuples and
# shapes (arrays and hashes, respectively, of known shape) to these
# two classes. All the actual methods on this class are implemented in
# C++.
class Sorbet::Private::Static::Tuple < Array
  extend T::Generic
  Elem = type_member(:out)

  def [](*args); end

  def last(*args); end
  def first(*args); end
  def min(*args); end
  def max(*args); end
  def to_a; end
  def concat(*arrays); end
end

class Sorbet::Private::Static::Shape < Hash
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)

  def merge(other); end
  def to_hash(); end
end

class Sorbet::Private::Static::ENVClass
  extend T::Generic
  include Enumerable
  Elem = type_member(:out, fixed: [String, T.nilable(String)])

  sig do
    params(
        key: String,
    )
    .returns(T.nilable(String))
  end
  def [](key); end

  sig do
    params(
        key: String,
        value: T.nilable(String),
    )
    .returns(T.nilable(String))
  end
  def []=(key, value); end

  sig {returns(Sorbet::Private::Static::ENVClass)}
  def clear(); end

  sig do
    params(
        key: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        key: String,
        blk: T.proc.params(key: String).returns(T.untyped)
    )
    .returns(T.nilable(String))
  end
  def delete(key, &blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig {returns(T::Enumerator[Elem])}
  def delete_if(&blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig {returns(T::Enumerator[Elem])}
  def each(&blk); end

  sig do
    params(
        blk: T.proc.params(name: String).returns(BasicObject),
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig {returns(T::Enumerator[Elem])}
  def each_key(&blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig {returns(T::Enumerator[Elem])}
  def each_pair(&blk); end

  sig do
    params(
        blk: T.proc.params(value: String).returns(BasicObject),
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig {returns(T::Enumerator[Elem])}
  def each_value(&blk); end

  sig {returns(T::Boolean)}
  def empty?(); end

  sig do
    params(
        key: String,
    )
    .returns(String)
  end
  sig do
    type_parameters(:U)
    .params(
        key: String,
        value: T.type_parameter(:U),
    )
    .returns(T.any(String, T.type_parameter(:U)))
  end
  sig do
    type_parameters(:U)
    .params(
        key: String,
        blk: T.proc.params(key: String).returns(T.type_parameter(:U)),
    )
    .returns(T.any(String, T.type_parameter(:U)))
  end
  def fetch(key, value=T.unsafe(nil), &blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(T::Hash[String, String])
  end
  sig {returns(T::Enumerator[Elem])}
  def filter(&blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(T.nilable(Sorbet::Private::Static::ENVClass))
  end
  sig {returns(T::Enumerator[Elem])}
  def filter!(&blk); end

  sig do
    params(
        key: String
    )
    .returns(T::Boolean)
  end
  def has_key?(key); end

  sig do
    params(
        value: String
    )
    .returns(T::Boolean)
  end
  def has_value?(value); end

  sig do
    params(
        key: String
    )
    .returns(T::Boolean)
  end
  def include?(key); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig {returns(T::Enumerator[Elem])}
  def keep_if(&blk); end

  sig do
    params(
        name: String,
    )
    .returns(T.nilable(String))
  end
  def key(name); end

  sig do
    params(
        key: String
    )
    .returns(T::Boolean)
  end
  def key?(key); end

  sig do
    returns(T::Array[String])
  end
  def keys; end

  sig {returns(Integer)}
  def length(); end

  sig {void}
  def rehash(); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(T::Hash[String, String])
  end
  sig {returns(T::Enumerator[Elem])}
  def reject(&blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(T.nilable(Sorbet::Private::Static::ENVClass))
  end
  sig {returns(T::Enumerator[Elem])}
  def reject!(&blk); end

  sig do
    params(
        other: T.any(Sorbet::Private::Static::ENVClass, T::Hash[String, T.nilable(String)])
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  def replace(other); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(T::Hash[String, String])
  end
  sig {returns(T::Enumerator[Elem])}
  def select(&blk); end

  sig do
    params(
        blk: T.proc.params(name: String, value: String).returns(BasicObject),
    )
    .returns(T.nilable(Sorbet::Private::Static::ENVClass))
  end
  sig {returns(T::Enumerator[Elem])}
  def select!(&blk); end

  sig do
    returns(T::Hash[String, T.nilable(String)])
  end
  def to_hash; end

  sig do
    params(
        key: T::Hash[String, T.nilable(String)],
    )
    .returns(Sorbet::Private::Static::ENVClass)
  end
  sig do
    params(
        key: T::Hash[String, String],
        blk: T.proc.params(key: String, old_value: T.nilable(String), new_value: T.nilable(String)).returns(T.nilable(String)),
    )
    .returns(T::Hash[String, T.nilable(String)])
  end
  def update(key, &blk); end

  sig do
    params(
        value: String
    )
    .returns(T::Boolean)
  end
  def value?(value); end
end
# [`ENV`](https://docs.ruby-lang.org/en/2.6.0/ENV.html) is a hash-like accessor
# for environment variables.
::ENV = T.let(T.unsafe(nil), Sorbet::Private::Static::ENVClass)

# The magic type that sig {void} returns
class Sorbet::Private::Static::Void
end

class Sorbet::Private::Static::ReturnTypeInference
    extend T::Sig

    sig do
        type_parameters(:INFERRED_RETURN_TYPE, :INFERRED_ARGUMENT_TYPE)
        .params(a: T.type_parameter(:INFERRED_ARGUMENT_TYPE))
        .returns(T.type_parameter(:INFERRED_RETURN_TYPE))
    end
    def guessed_type_type_parameter_holder(a)
    # this method only exists to define the type parameter.
    # it's used in infer for return type inference.
    end
end

# Type alias for file-like objects. Many, but not all, file-like
# types in the Ruby stdlib are descendants of IO. These include
# pipes and sockets. These descendants are intentionally omitted
# here.
::Sorbet::Private::Static::IOLike = T.type_alias do
  T.any(
    IO,
    StringIO
  )
end

# These are extensions to the Ruby language that we added to make the type systems easier
# typed: strict

class Struct
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
    )
    .returns(T.class_of(Sorbet::Private::Static::DynamicStruct))
  end
  def self.new(arg0, *arg1); end

  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end

  sig {returns(T::Array[Symbol])}
  def self.members; end
end

module Sorbet::Private::Static
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

class Sorbet::Private::Static::DynamicStruct < Struct
  Elem = type_member(:out, fixed: T.untyped)

  sig do
    params(
        args: BasicObject,
    )
    .returns(Sorbet::Private::Static::DynamicStruct)
  end
  def self.new(*args); end
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
        key: String
    )
    .returns(T::Boolean)
  end
  def key?(key); end

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
end
::ENV = T.let(T.unsafe(nil), Sorbet::Private::Static::ENVClass)

# The magic type that sig {void} returns
module Sorbet::Private::Static::Void
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
::Sorbet::Private::Static::IOLike = T.type_alias(
  T.any(
    File,
    IO,
    StringIO,
    Tempfile
  )
)

# These are extensions to the Ruby language that we added to make the type systems easier
# typed: strict

class Struct
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
    )
    .returns(T.class_of(RubyTyper::DynamicStruct))
  end
  def self.new(arg0, *arg1); end
end

module RubyTyper
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
end

class RubyTyper::DynamicStruct < Struct
  Elem = type_member(:out, fixed: T.untyped)

  sig do
    params(
        args: BasicObject,
    )
    .returns(RubyTyper::DynamicStruct)
  end
  def self.new(*args); end
end

class RubyTyper::StubClass
end

class RubyTyper::ImplicitModuleSuperclass < BasicObject
end

# C++ code delegates the implementation of many methods on tuples and
# shapes (arrays and hashes, respectively, of known shape) to these
# two classes. All the actual methods on this class are implemented in
# C++.
class RubyTyper::Tuple < Array
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

class RubyTyper::Shape < Hash
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)

  def merge(other); end
end

class RubyTyper::ENVClass
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

  sig {returns(RubyTyper::ENVClass)}
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
    params(
        key: String,
        value: T.nilable(String),
    )
    .returns(String)
  end
  sig do
    params(
        key: String,
        blk: T.proc.params(key: String).returns(String),
    )
    .returns(String)
  end
  def fetch(key, value=T.unsafe(nil), &blk); end

  sig do
    params(
        key: String
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def key?(key); end

  sig do
    params(
        key: T::Hash[String, T.nilable(String)],
    )
    .returns(RubyTyper::ENVClass)
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
::ENV = T.let(T.unsafe(nil), RubyTyper::ENVClass)

# The magic type that sig {void} returns
module RubyTyper::Void
end

class RubyTyper::ReturnTypeInference
    extend T::Helpers

    sig {type_parameters(:INFERRED_RETURN_TYPE).params().returns(T.type_parameter(:INFERRED_RETURN_TYPE))}
    def guessed_type_type_parameter_holder
    # this method only exists to define the type parameter.
    # it's used in infer for return type inference.
    end
end

# Type alias for file-like objects. Many, but not all, file-like
# types in the Ruby stdlib are descendants of IO. These include
# pipes and sockets. These descendants are intentionally omitted
# here.
::RubyTyper::IOLike = T.type_alias(
  T.any(
    File,
    IO,
    StringIO,
    Tempfile
  )
)

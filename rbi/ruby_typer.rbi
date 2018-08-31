# These are extensions to the Ruby language that we added to make the type systems easier
# typed: strict

class Struct
  Sorbet.sig(
      arg0: T.any(Symbol, String),
      arg1: T.any(Symbol, String),
  )
  .returns(T.class_of(RubyTyper::DynamicStruct))
  def self.new(arg0, *arg1); end
end

module RubyTyper
  Sorbet.sig(
      expr: T.untyped,
  )
  .void
  def self.keep_for_ide(expr)
  end

  Sorbet.sig(
      expr: T.untyped,
  )
  .void
  def self.keep_for_typechecking(expr)
  end
end

class RubyTyper::DynamicStruct < Struct
  Elem = type_member(:out, fixed: T.untyped)

  Sorbet.sig(
      args: BasicObject,
  )
  .returns(RubyTyper::DynamicStruct)
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

  Sorbet.sig(
      key: String,
  )
  .returns(T.nilable(String))
  def [](key); end

  Sorbet.sig(
      key: String,
      value: T.nilable(String),
  )
  .returns(T.nilable(String))
  def []=(key, value); end

  Sorbet.sig.returns(RubyTyper::ENVClass)
  def clear(); end

  Sorbet.sig(
      key: String,
  )
  .returns(T.nilable(String))
  Sorbet.sig(
      key: String,
      blk: T.proc(key: String).returns(T.untyped)
  )
  .returns(T.nilable(String))
  def delete(key, &blk); end

  Sorbet.sig(
      key: String,
  )
  .returns(String)
  Sorbet.sig(
      key: String,
      value: T.nilable(String),
  )
  .returns(String)
  Sorbet.sig(
      key: String,
      blk: T.proc(key: String).returns(String),
  )
  .returns(String)
  def fetch(key, value=T.unsafe(nil), &blk); end

  Sorbet.sig(
      key: String
  )
  .returns(T.any(TrueClass, FalseClass))
  def key?(key); end

  Sorbet.sig(
      key: T::Hash[String, T.nilable(String)],
  )
  .returns(RubyTyper::ENVClass)
  Sorbet.sig(
      key: T::Hash[String, String],
      blk: T.proc(key: String, old_value: T.nilable(String), new_value: T.nilable(String)).returns(T.nilable(String)),
  )
  .returns(T::Hash[String, T.nilable(String)])
  def update(key, &blk); end
end
::ENV = T.let(T.unsafe(nil), RubyTyper::ENVClass)

# The magic type that sig.void returns
module RubyTyper::Void
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

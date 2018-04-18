# These are extensions to the Ruby language that we added to make the type systems easier

class Struct
  sig(
      arg0: T.any(Symbol, String),
      arg1: T.any(Symbol, String),
  )
  .returns(RubyTyper::DynamicStruct.singleton_class)
  def self.new(arg0, *arg1); end
end

class RubyTyper::DynamicStruct < Struct
  Elem = type_member(fixed: T.untyped)

  sig(
      args: BasicObject,
  )
  .returns(RubyTyper::DynamicStruct)
  def self.new(*args); end
end

class RubyTyper::StubClass
end

class RubyTyper::ENVClass
  extend T::Generic
  include Enumerable
  Elem = type_member(fixed: [String, T.nilable(String)])

  sig(
      key: String,
  )
  .returns(T.nilable(String))
  def [](key); end

  sig(
      key: String,
      value: T.nilable(String),
  )
  .returns(T.nilable(String))
  def []=(key, value); end

  sig.returns(RubyTyper::ENVClass)
  def clear(); end

  sig(
      key: String,
  )
  .returns(T.nilable(String))
  sig(
      key: String,
      blk: T.proc(key: String).returns(T.untyped)
  )
  .returns(T.nilable(String))
  def delete(key, &blk); end

  sig(
      key: String,
  )
  .returns(String)
  sig(
      key: String,
      value: T.nilable(String),
  )
  .returns(String)
  sig(
      key: String,
      blk: T.proc(key: String).returns(String),
  )
  .returns(String)
  def fetch(key, value=_, &blk); end

  sig(
      key: String
  )
  .returns(T.any(TrueClass, FalseClass))
  def key?(key); end

  sig(
      key: T::Hash[String, T.nilable(String)],
  )
  .returns(RubyTyper::ENVClass)
  sig(
      key: T::Hash[String, String],
      blk: T.proc(key: String, old_value: T.nilable(String), new_value: T.nilable(String)).returns(T.nilable(String)),
  )
  .returns(T::Hash[String, T.nilable(String)])
  def update(key, &blk); end
end
::ENV = T.cast(nil, RubyTyper::ENVClass)

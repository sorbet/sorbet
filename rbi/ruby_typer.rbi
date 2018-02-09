# These are extensions to the Ruby language that we added to make the type systems easier

class Struct
  Elem = type_member(fixed: T.untyped)

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
  def self.new(*arg1); end
end

class RubyTyper::StubClass
end

# typed: true
extend T::Sig

class MyObject
  extend T::Sig

  sig do
    type_parameters(:U)
    .params(
      blk: T.proc.params(arg0: T.self_type).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def my_yield_self(&blk)
    yield self
  end

  sig { params(blk: T.proc.params(arg0: T.self_type).void).returns(T.self_type) }
  def my_tap(&blk)
    yield self
    self
  end
end

class MyChild < MyObject
  def child_only; end
end

MyObject.new.my_yield_self do |this|
  T.reveal_type(this) # error: `MyObject`
end

T.reveal_type(MyObject.new.my_yield_self {|this| this.to_s}) # error: `String`

# `T.self_type` tracks the runtime receiver, not the class the method was defined in.
MyChild.new.my_yield_self do |this|
  T.reveal_type(this) # error: `MyChild`
  this.child_only
end

T.reveal_type(MyChild.new.my_tap {|this| this.child_only}) # error: `MyChild`

# Same thing, but for the versions that live in Sorbet's RBI files for the stdlib.
T.reveal_type("".then {|s| s.upcase}) # error: `String`
T.reveal_type(0.yield_self {|i| i.even?}) # error: `T::Boolean`
"".tap do |s|
  T.reveal_type(s) # error: `String("")`
end

class Generic
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig { params(blk: T.proc.params(arg0: T.self_type).void).void }
  def each_self(&blk)
    yield self
  end
end

Generic[Integer].new.each_self do |this|
  T.reveal_type(this) # error: `Generic[Integer]`
end

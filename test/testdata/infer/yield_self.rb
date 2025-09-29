# typed: true
extend T::Sig

class MyObject
  extend T::Sig

  sig do
    type_parameters(:U)
    .params(
      blk: T.proc.params(arg0: T.self_type).returns(T.type_parameter(:U)),
      #                        ^^^^^^^^^^^ error: Only top-level `T.self_type` is supported
    )
    .returns(T.type_parameter(:U))
  end
  def my_yield_self(&blk)
    yield self
  end
end

MyObject.new.my_yield_self do |this|
  T.reveal_type(this) # error: `T.untyped`
end

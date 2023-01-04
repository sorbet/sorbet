# typed: true

class A
  extend T::Sig

  sig { params(arg0: T.untyped, arg1: T.untyped, arg2: T.untyped, arg3: T.untyped).returns(T::Array[T.untyped]) }
  #            ^^^^ usage: arg0
  def example1(arg0, arg1 = nil, arg2:, arg3: nil)
    #          ^^^^ def: arg0
    [arg0, arg1, arg2, arg3]
    #^^^^ usage: arg0
  end
end

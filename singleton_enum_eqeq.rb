# typed: strict
extend T::Sig

module MyEnum
  extend T::Helpers
  sealed!

  class A_
    include Singleton
    include MyEnum
  end
  A = T.let(A_.instance, A_)

  class B_
    include Singleton
    include MyEnum
  end
  B = T.let(B_.instance, B_)

  class C_
    include Singleton
    include MyEnum
  end
  C = T.let(C_.instance, C_)
end



sig {params(x: MyEnum).void}
def foo(x)
  if x == MyEnum::A
    T.reveal_type(x)
  elsif x == MyEnum::B
  elsif x == MyEnum::C
  else
    T.absurd(x)
  end
end


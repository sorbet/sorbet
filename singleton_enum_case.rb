# typed: strict
extend T::Sig

class MyEnum
  extend T::Helpers
  sealed!
  abstract!

  class JakesFancyThing < MyEnum; extend T::Helpers; include Singleton; final!; end
  A = T.let(JakesFancyThing.instance, JakesFancyThing)

  class B_ < MyEnum; extend T::Helpers; include Singleton; final!; end
  B = T.let(B_.instance, B_)

  class C_ < MyEnum; extend T::Helpers; include Singleton; final!; end
  C = T.let(C_.instance, C_)
end


sig {params(x: MyEnum::A).void}
def foo(x)

end

sig {params(x: MyEnum).void}
def bar(x)
  case x
  when MyEnum::A
    T.reveal_type(x)
    foo(x)
  when MyEnum::B
  # when MyEnum::C
  else
    T.absurd(x)
  end
end

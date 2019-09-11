# typed: strict
extend T::Sig

class MyEnum
  extend T::Helpers
  sealed!
  abstract!

  class AType < MyEnum; include Singleton; final!; end
  A = T.let(AType.instance, AType)

  class BType < MyEnum; include Singleton; final!; end
  B = T.let(BType.instance, BType)

  class CType < MyEnum; include Singleton; final!; end
  C = T.let(CType.instance, CType)
end

sig {params(x: MyEnum).void}
def all_cases_handled(x)
  case x
  when MyEnum::A
    T.reveal_type(x) # error: Revealed type: `MyEnum::AType`
  when MyEnum::B
    T.reveal_type(x) # error: Revealed type: `MyEnum::BType`
  when MyEnum::C
    T.reveal_type(x) # error: Revealed type: `MyEnum::CType`
  else
    T.absurd(x)
  end
end

sig {params(x: MyEnum).void}
def missing_case(x)
  case x
  when MyEnum::A
  when MyEnum::B
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `MyEnum::CType` wasn't handled
  end
end

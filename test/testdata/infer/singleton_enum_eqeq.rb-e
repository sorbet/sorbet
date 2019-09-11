# typed: strict
extend T::Sig

module MyEnum
  extend T::Helpers
  sealed!

  class AType
    extend T::Helpers
    include Singleton
    include MyEnum
    final!
  end
  A = T.let(AType.instance, AType)

  class BType
    extend T::Helpers
    include Singleton
    include MyEnum
    final!
  end
  B = T.let(BType.instance, BType)

  class CType
    extend T::Helpers
    include Singleton
    include MyEnum
    final!
  end
  C = T.let(CType.instance, CType)
end

sig {params(x: MyEnum).void}
def all_cases_handled(x)
  if x == MyEnum::A
    T.reveal_type(x) # error: Revealed type: `MyEnum::AType`
  elsif x == MyEnum::B
    T.reveal_type(x) # error: Revealed type: `MyEnum::BType`
  elsif x == MyEnum::C
    T.reveal_type(x) # error: Revealed type: `MyEnum::CType`
  else
    T.absurd(x)
  end
end

sig {params(x: MyEnum).void}
def missing_case(x)
  if x == MyEnum::A
  elsif x == MyEnum::B
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `MyEnum::CType` wasn't handled
  end
end


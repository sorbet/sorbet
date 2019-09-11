# typed: strict
extend T::Sig

class MyEnum
  extend T::Helpers
  sealed!
  abstract!

  class AType < MyEnum; include Singleton; end
  A = T.let(AType.instance, AType)

  class BType < MyEnum; include Singleton; end
  B = T.let(BType.instance, BType)

  class CType < MyEnum; include Singleton; end
  C = T.let(CType.instance, CType)
end

sig {params(x: MyEnum).void}
def if_over_non_final_singletons(x)
  if x == MyEnum::A
    T.reveal_type(x) # error: Revealed type: `MyEnum::AType`
  elsif x == MyEnum::B
    T.reveal_type(x) # error: Revealed type: `MyEnum::BType`
  elsif x == MyEnum::C
    T.reveal_type(x) # error: Revealed type: `MyEnum::CType`
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `MyEnum` wasn't handled
  end
end

# This currently differs from the `==` behavior above for no real reason, and
# could probably be changed.
sig {params(x: MyEnum).void}
def case_over_non_final_singletons(x)
  case x
  when MyEnum::A
    T.reveal_type(x) # error: Revealed type: `MyEnum`
  when MyEnum::B
    T.reveal_type(x) # error: Revealed type: `MyEnum`
  when MyEnum::C
    T.reveal_type(x) # error: Revealed type: `MyEnum`
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `MyEnum` wasn't handled
  end
end


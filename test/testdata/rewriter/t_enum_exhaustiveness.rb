# typed: strict

extend T::Sig

class MyEnum < T::Enum
  enums do
  A = new
  B = new
  C = new
  end
end

sig {params(x: MyEnum).void}
def all_cases_handled(x)
  case x
  when MyEnum::A
    T.reveal_type(x) # error: Revealed type: `MyEnum::A`
  when MyEnum::B
    T.reveal_type(x) # error: Revealed type: `MyEnum::B`
  when MyEnum::C
    T.reveal_type(x) # error: Revealed type: `MyEnum::C`
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
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `MyEnum::C` wasn't handled
  end
end

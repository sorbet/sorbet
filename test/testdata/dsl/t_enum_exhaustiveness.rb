# typed: strict

extend T::Sig

module T
  class Enum
    extend T::Sig

    sig {params(x: T.nilable(String)).void}
    def initialize(x = nil)
    end

    sig {params(blk: T.proc.void).void}
    def self.enums(&blk)
    end
  end
end

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

# typed: strict
# disable-fast-path: true
extend T::Sig

module Opus
  class Enum
    extend T::Sig
    extend T::Generic

    sig {params(x: T.nilable(String)).void}
    def initialize(x = nil)
    end
  end
end

class MyEnum < Opus::Enum
  A = new
  B = new
  C = new
  D = new
end

sig {params(x: T.any(MyEnum::A, MyEnum::B)).void}
def takes_a_or_b(x); end

sig {params(x: MyEnum::C).void}
def takes_c(x); end

sig {params(x: MyEnum).void}
def some_common_cases(x)
  case x
  when MyEnum::A, MyEnum::B
    takes_a_or_b(x)
  when MyEnum::C
    takes_c(x)
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `MyEnum::D` wasn't handled
  end
end

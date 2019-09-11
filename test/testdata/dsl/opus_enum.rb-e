# typed: strict
# disable-fast-path: true
module Opus
  class Enum
    extend T::Sig
    extend T::Generic
    extend Enumerable

    Elem = type_template

    sig {params(x: T.nilable(String)).void}
    def initialize(x = nil)
    end

    sig {override.params(blk: T.untyped).returns(T.untyped)}
    def self.each(&blk); end
  end
end

class MyEnum < Opus::Enum
  Elem = type_template

  X = new
  Y = new('y')
  Z = T.let(new, self)
end

T.reveal_type(MyEnum::X) # error: Revealed type: `MyEnum::X`
T.reveal_type(MyEnum::Y) # error: Revealed type: `MyEnum::Y`
T.reveal_type(MyEnum::Z) # error: Revealed type: `MyEnum::Z`

class NotAnEnum
  X = new # error: Constants must have type annotations
  Y = T.let(new, self)
end

T.reveal_type(NotAnEnum::X) # error: Revealed type: `T.untyped`
T.reveal_type(NotAnEnum::Y) # error: Revealed type: `NotAnEnum`

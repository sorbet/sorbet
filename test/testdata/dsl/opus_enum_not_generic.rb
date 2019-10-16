# typed: strict

module Opus
  class Enum
    extend T::Sig

    sig {params(x: T.nilable(String)).void}
    def initialize(x = nil)
    end

    sig {params(blk: T.proc.void).void}
    def self.enums(&blk); end
  end
end

class MyEnum < Opus::Enum
  enums do
  X = new
  end
end

T.reveal_type(MyEnum::X) # error: Revealed type: `MyEnum::X`

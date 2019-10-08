# typed: true

class EnumsDoEnum < Opus::Enum
  enums do
    Elem = type_template(fixed: self)
    X = new(:failing_send)
  end
end

module Opus
  class Enum
    extend T::Generic
    Elem = type_template
    extend Enumerable
    def initialize(x = nil)
    end

    def self.each(&blk)
    end
  end
end

T.reveal_type(EnumsDoEnum.first) # error: Revealed type: `T.nilable(EnumsDoEnum)`

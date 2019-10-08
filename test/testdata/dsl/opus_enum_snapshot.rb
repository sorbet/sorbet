# typed: true
module Opus
  class Enum
    extend T::Generic
    def initialize(x = nil)
    end
    def self.enums(&blk)
    end
  end
end

class MyEnum < Opus::Enum
  enums do
  X = new
  Y = new('y')
  Z = T.let(new, self)
  end
end

class NotAnEnum
  enums do # error: does not exist
  X = new
  Y = T.let(new, self)
  end
end

class EnumsDoEnum < Opus::Enum
  enums do
    X = new
    Y = new('y')
    Z = T.let(new, self)
  end

  def something_outside; end
  SomethingElseOutside = 1
end

class BadConsts < Opus::Enum
  Before = new # error: must be within the `enums do` block
  enums do
    Inside = new
  end
  After = new # error: must be within the `enums do` block
end

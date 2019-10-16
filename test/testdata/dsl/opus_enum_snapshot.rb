# typed: true
module Opus
  class Enum
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
end

class BadConsts < Opus::Enum
  Before = new # error: must be within the `enums do` block
  StaticField1 = 1 # error: must be unique instances of the enum
  enums do
    Inside = new
    StaticField2 = 2 # error: must be unique instances of the enum
  end
  After = new # error: must be within the `enums do` block
  StaticField3 = 3 # error: must be unique instances of the enum
  StaticField4 = T.let(1, Integer) # error: must be unique instances of the enum
end

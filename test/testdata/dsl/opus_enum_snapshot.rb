# typed: true
module Opus
  class Enum
    extend T::Generic
    def initialize(x = nil)
    end
  end
end

class MyEnum < Opus::Enum
  X = new
  Y = new('y')
  Z = T.let(new, self)
end

class NotAnEnum
  X = new
  Y = T.let(new, self)
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

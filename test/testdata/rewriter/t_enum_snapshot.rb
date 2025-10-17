# typed: true
class MyEnum < T::Enum
  enums do
  X = new
  Y = new('y')
  Z = T.let(new, self)
  end
end

class NotAnEnum
  enums do # error: does not exist
  X = new
  Y = T.let(new, self) # error: Unsupported type syntax
  end
end

class EnumsDoEnum < T::Enum
  enums do
    X = new
    Y = new('y')
    Z = T.let(new, self)
  end

  def something_outside; end
end

class BadConsts < T::Enum
  Before = new # error: Definition of enum value `Before` must be within the `enums do` block for this `T::Enum`
  StaticField1 = 1 # error: All non-enum constants in a `T::Enum` must be defined after the `enums do` block
  enums do
    Inside = new
    StaticField2 = 2 # error: All non-enum constants in a `T::Enum` must be defined after the `enums do` block
  end
  After = new # error: must be within the `enums do` block
  StaticField3 = 3
  StaticField4 = T.let(1, Integer)
end

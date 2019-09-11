# typed: true

class A1
  extend T::Generic
  TM = type_member

  def self.foo
    # should fail: TM is used in a static context
    T::Array[TM].new
  end
end

class A2
  extend T::Generic
  TM = type_member

  # should fail: TM is used in a static context
  s = T::Array[TM].new
end

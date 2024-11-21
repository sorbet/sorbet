# typed: true

class A
  extend T::Generic

  X = type_member(:out, :extra) # error: Too many args in type definition
  Y = type_member # This is valid - no args
  Z = type_member(:out) # This is valid - one arg
end

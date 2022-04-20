# typed: true

class B < A
  extend T::Generic
  X = type_template {{fixed: String}}
  #                          ^^^^^^ error: The `fixed` type bound `String` must be equivalent to the parent's `fixed` type bound `Integer` for type_template `X`
  #                          ^^^^^^ error: The `fixed` type bound `String` must be equivalent to the parent's `fixed` type bound `Integer` for type_template `X`
end

class A
  extend T::Generic
  X = type_template {{fixed: Integer}}
end

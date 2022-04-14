# typed: true
# disable-fast-path: true

class B < A
  extend T::Generic
  X = type_template {{fixed: String}}
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: parent lower bound `Integer`
    # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: upper bound `String`
end

class A
  extend T::Generic
  X = type_template {{fixed: Integer}}
end

# typed: true

class A
  extend T::Generic

  X = type_member
  X = type_member
# ^ error: Duplciate type member `X`
  X = type_member
# ^ error: Duplciate type member `X`
end

class B
  extend T::Generic

  Y = type_template
  Y = type_template
# ^ error: Duplciate type member `Y`
  Y = type_template
# ^ error: Duplciate type member `Y`
end

# typed: true

class A
  extend T::Generic

  X = type_member
  X = type_member
# ^ error: Duplicate type member `X`
  X = type_member
# ^ error: Duplicate type member `X`
end

class B
  extend T::Generic

  Y = type_template
  Y = type_template
# ^ error: Duplicate type template `Y`
  Y = type_template
# ^ error: Duplicate type template `Y`
end

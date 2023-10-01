# typed: true

class Example1
  extend T::Generic
  X = type_member { {upper: X} }
end

class Example2
  extend T::Generic
  X = type_member { {upper: Y} }
  Y = type_member { {upper: X} }
end

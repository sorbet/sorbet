# typed: strong

class Foo < T::Struct
  prop :a, Integer
end

class Bar
  include T::Props

  prop :b, Integer
end

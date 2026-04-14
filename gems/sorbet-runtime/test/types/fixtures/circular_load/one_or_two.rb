# typed: true

module Opus::Types::Test::Fixtures::CircularLoad
  OneOrTwo = T.type_alias do
    T.any(
      T.class_of(Child1),
      T.class_of(Child2)
    )
  end
end

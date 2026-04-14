# typed: true

module Opus::Types::Test::Fixtures::CircularLoad
  class Child2 < Parent
    Parent.foo(self)
  end
end

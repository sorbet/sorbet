# typed: true

module Opus::Types::Test::Fixtures::CircularLoad
  class Child1 < Parent
    foo(self)
  end
end

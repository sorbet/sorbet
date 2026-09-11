# typed: true

module Root::A # error: `Root::A` belongs to package `Root::A`
  module Test
    class FooTest
    end
  end
end

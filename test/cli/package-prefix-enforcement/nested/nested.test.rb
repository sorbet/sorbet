# typed: strict

extend T::Sig

class ::UnitTest; end

# Correct location
module Test::Root
  NOT_IN_MODULE = nil
  module Nested
    class MyTest < UnitTest
    end
  end
end

module Root::Nested::ShouldBeInTestPrefix
end

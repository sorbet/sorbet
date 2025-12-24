# typed: strict

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

Test::Root::X = 1
Test::Root::Nested::X = 1
Test::Root = 1
Test::Root::Nested = 1

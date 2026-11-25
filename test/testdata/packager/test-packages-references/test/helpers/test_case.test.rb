# typed: true

module Helpers::Test
  class TestCase
    def test_a
      Root::A.new
    # ^^^^ importusage: rootpkg

      Root::Test::B.new
    # ^^^^^^^^^^ error: Unable to resolve constant `Test`
    # ^^^^ importusage: rootpkg
    end
  end
end

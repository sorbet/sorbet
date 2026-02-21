# typed: true

module Test::Helpers
  class TestCase
    def test_a
      Root::A.new
    # ^^^^ importusage: rootpkg

      Test::Root::B.new
    # ^^^^^^^^^^ error: Unable to resolve constant `Root`
    end
  end
end

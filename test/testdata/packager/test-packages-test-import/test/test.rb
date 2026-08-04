# typed: true

module Test::Root
  class ATest
    def test_a
      Root::A.new
    # ^^^^ error: `Root` is not imported
    end
  end
end

# typed: true

module Root::Test
  class ATest
    def test_a
      Root::A.new # error: `Root::A` resolves but its package is not imported
    end
  end
end

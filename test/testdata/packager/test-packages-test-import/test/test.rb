# typed: true

module Root::Test
  class ATest
    def test_a
      Root::A.new # error: `Root` is not imported
    end
  end
end

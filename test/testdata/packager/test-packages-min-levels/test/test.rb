# typed: strict

module Root::Test
  class ATest
    extend T::Sig
    sig { void }
    def test_a
      Root::A.new
    end
  end
end

# typed: true

module Test::Family
  module Bart
    class BartTest
      extend T::Sig

      # This should resolve to Test::Family::TestFamily
      sig {params(x: TestFamily).void}
      def test1(x)
      end

      # This should be stubbed as Test::Family::Bart::Slingshot::TestSlingshot
      sig {params(x: Slingshot::TestSlingshot).void}
      def test2(x)
      end
    end
  end
end

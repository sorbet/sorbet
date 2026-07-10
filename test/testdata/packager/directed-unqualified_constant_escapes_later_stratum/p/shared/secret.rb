# typed: true
# stratum: 2

module P
  module Shared
    class Secret
      def base
        Base::Thing.new
      end
    end
  end
end

# typed: true
# stratum: 2

module P # error: `P` may not be opened
  module Shared
    class Secret
      def base
        Base::Thing.new
      end
    end
  end
end

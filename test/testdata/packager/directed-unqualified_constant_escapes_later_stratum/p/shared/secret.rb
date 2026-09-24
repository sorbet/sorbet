# typed: true
# stratum: 2

module P # error: Package `P::Shared` may not open `P`
  module Shared
    class Secret
      def base
        Base::Thing.new
      end
    end
  end
end

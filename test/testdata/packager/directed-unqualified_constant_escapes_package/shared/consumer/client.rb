# typed: true
# stratum: 0

module Shared
  module Consumer
    class Client
      def call
        # Identical to `unqualified_constant_escapes_package`, but in package-directed mode. Both
        # files sit in stratum 0, so `Shared::Secret` resolves the same way (the lexical walk climbs
        # into the parent package's `Shared` namespace). Package-directed mode changes how strata are
        # named and resolved, not the cross-package visibility rule, so the same error still fires.
        Secret
      # ^^^^^^ error: `Shared::Secret` resolves but is not exported from `Shared` and `Shared` is not imported
      end
    end
  end
end

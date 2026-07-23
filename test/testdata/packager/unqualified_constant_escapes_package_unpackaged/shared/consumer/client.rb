# typed: true

module Shared
  module Consumer
    class Client
      def call
        # Identical code to the packaged variants, but with no `__package.rb` files and no
        # `enable-packager`, so the packager never runs. Constant resolution is unchanged -- the
        # lexical walk still climbs into `Shared` and finds `Secret` -- but with no package boundary
        # to enforce, the reference resolves cleanly and no error is reported. This isolates the
        # boundary check as a packager-phase concern, not a resolver-phase one.
        Secret
      end
    end
  end
end

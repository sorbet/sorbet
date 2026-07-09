# typed: true

module Shared
  module Consumer
    class Client
      def call
        # Unqualified `Secret` is not defined in this package's namespace `Shared::Consumer`.
        # Because this file nests `module Shared` around `module Consumer`, the parent namespace
        # `Shared` is a lexical scope here, so constant resolution walks Client -> Consumer ->
        # Shared and finds `Shared::Secret`, which belongs to the *parent* package `Shared`. The
        # resolver never stops at package boundaries; the packager then flags the cross-package
        # reference because `Shared` is neither imported here nor exports `Secret`.
        Secret
      # ^^^^^^ error: `Shared::Secret` resolves but is not exported from `Shared` and `Shared` is not imported
      end
    end
  end
end

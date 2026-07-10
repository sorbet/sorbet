# typed: true
# stratum: 1

module P
  module Shared
    module Consumer
      class Client
        extend T::Sig

        def call
          # `Secret` is only ever defined as `P::Shared::Secret`, which lives in the later `P::Shared`
          # stratum (stratum 2). At this stratum-1 file's resolution it does not exist yet, and neither
          # `P` nor the root scope defines a `Secret`, so the unqualified walk finds nothing.
          Secret
        # ^^^^^^ error: Unable to resolve constant `Secret`

          # `Secret2` is defined in *two* places along the lexical walk: `P::Shared::Secret2` (the
          # intended, closer one, in the later `P::Shared` stratum 2) and `P::Secret2` (the outer one,
          # in `P` at stratum 0, which this package imports). Because the closer `P::Shared::Secret2`
          # has not been resolved yet at this stratum, the walk climbs past `Shared` and binds to the
          # outer `P::Secret2` -- so this resolves with no error, but to the *wrong* constant. In a
          # non-directed build `P::Shared::Secret2` would already exist and shadow `P::Secret2`.
          T.reveal_type(Secret2)
        # ^^^^^^^^^^^^^^^^^^^^^^ error: Revealed type: `T.class_of(P::Secret2)`
        end
      end
    end
  end
end

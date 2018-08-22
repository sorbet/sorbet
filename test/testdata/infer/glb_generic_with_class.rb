# typed: strict

extend T::Helpers

a = T.cast(nil, T.all(Array, T.nilable(Regexp)))
T.reveal_type(a) # we have a bug right now but this should be: error: Revealed type: `<Type: <impossible>>`

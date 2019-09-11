# typed: true

extend T::Sig

a = T.cast(nil, T.all(Array, T.nilable(Regexp)))
T.reveal_type(a) # error: This code is unreachable

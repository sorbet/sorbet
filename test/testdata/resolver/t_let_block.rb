# typed: true

w = T.let(1) {T.nilable(Integer)}
T.reveal_type(w) # error: Revealed type: `T.nilable(Integer)`

x = T.cast('') {T.nilable(String)}
T.reveal_type(x) # error: Revealed type: `T.nilable(String)`

y = T.assert_type!('') {T.nilable(Float)} # error: Argument does not have asserted type `T.nilable(Float)`
T.reveal_type(y) # error: Revealed type: `T.nilable(Float)`

# TODO(jez) Update this test after rebasing on the `T.bind` branch.

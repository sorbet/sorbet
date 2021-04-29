# typed: true

expr = T.unsafe(nil)
MyType = T.type_alias {Integer}

# For these using **expr, I don't really care what happens as long as it doesn't crash.

T.let(**expr)
T.let(**expr) {MyType}
T.let(expr, **expr)
T.let(expr, **expr) {MyType}

T.let(checked: false, **expr)
T.let(checked: false, **expr) {MyType}
T.let(expr, checked: false, **expr) {MyType} # error: specific keys are unknown

# --- Neither block form nor positional form ---

T.let(expr) # error: No type provided to `T.let`
T.let(expr, checked: false) # error: No type provided to `T.let`

# --- Block form ---

T.let(expr, checked: false, garbage: true) {MyType} # error: Unrecognized keyword argument `garbage`

T.let(expr) {MyType} # error: Using `T.let` in block form requires `checked: false`

T.let(expr, garbage: true) {MyType} # error: Unrecognized keyword argument `garbage`

T.let(expr, checked: nil) {MyType} # error: Using `T.let` in block form requires `checked: false`
T.let(expr, checked: 1) {MyType} # error: Using `T.let` in block form requires `checked: false`
T.let(expr, checked: expr) {MyType} # error: Using `T.let` in block form requires `checked: false`
T.let(expr, checked: true) {MyType} # error: Using `T.let` in block form requires `checked: false`

x1 = T.let(1, checked: false) {T.nilable(Integer)}
T.reveal_type(x1) # error: type: `T.nilable(Integer)`

T.let('not an int', checked: false) {T.nilable(Integer)} # error: Argument does not have asserted type `T.nilable(Integer)`

# --- Positional form ---

T.let(expr, MyType) {MyType} # error: Passed type to `T.let` twice, via argument and block

# --- All cast names ---

_a = T.let(1) {T.nilable(Integer)} # error: requires `checked: false`
_b = T.cast(1) {T.nilable(Integer)} # error: requires `checked: false`
_c = T.assert_type!(1) {T.nilable(Integer)} # error: requires `checked: false`

w = T.let(1, checked: false) {T.nilable(Integer)}
T.reveal_type(w) # error: Revealed type: `T.nilable(Integer)`

x = T.cast('', checked: false) {T.nilable(String)}
T.reveal_type(x) # error: Revealed type: `T.nilable(String)`

y = T.assert_type!('', checked: false) {T.nilable(Float)} # error: does not have asserted type
T.reveal_type(y) # error: Revealed type: `T.nilable(Float)`

# TODO(jez) Update this test after rebasing on the `T.bind` branch.
